/*
  ==============================================================================

	AutomationKey.cpp
	Created: 11 Dec 2016 1:22:20pm
	Author:  Ben

  ==============================================================================
*/

AutomationKey::AutomationKey(int numDimensions, Array<float> minimumValues, Array<float> maximumValues) :
	BaseItem("Key",false),
	numDimensions(numDimensions)
{
	itemDataType = "AutomationKey";
	hideEditorHeader = true;

	position = addFloatParameter("Position", "Position of the key", 0);
	position->defaultUI = FloatParameter::TIME;

	for (int i = 0; i < numDimensions; i++)
	{
		FloatParameter* value = addFloatParameter("Value", "Value of the key", 0, minimumValues[i], maximumValues[i]);
		values.add(value);

		easings.add(nullptr);
	}

	easingType = addEnumParameter("EasingType", "Type of transition to the next key");

	easingType->addOption("Linear", Easing::LINEAR);
	easingType->addOption("Bezier", Easing::BEZIER);
	easingType->addOption("Hold", Easing::HOLD);
	easingType->addOption("Sine", Easing::SINE);

	easingType->setValueWithData(Easing::LINEAR);
	easingType->hideInEditor = true;
	easingType->forceSaveValue = true;

	canInspectChildContainers = false;

	setEasing(Easing::LINEAR);

	helpID = "AutomationKey";
}

AutomationKey::~AutomationKey()
{
}

void AutomationKey::clearRange()
{
	for (auto& v : values) v->clearRange();
}

void AutomationKey::setRange(Array<float> minVal, Array<float> maxVal)
{
	for (int i = 0; i < numDimensions; i++) values[i]->setRange(minVal[i], maxVal[i]);
}

void AutomationKey::setEasing(Easing::Type t)
{
	for (int i = 0; i < numDimensions; i++)
	{
		if (easings[i] != nullptr)
		{
			if (easings[i]->type == t) return;
			removeChildControllableContainer(easings[i]);
		}

		Easing* e = nullptr;
		switch (t)
		{
		case Easing::LINEAR:
			e = new LinearEasing();
			break;

		case Easing::HOLD:
			e = new HoldEasing();
			break;

		case Easing::BEZIER:
			e = new CubicEasing();
			break;

		case Easing::SINE:
			e = new  SineEasing();
			break;
		}

		easings.set(i, e);

		if (easings[i] != nullptr)
		{
			easings[i]->setSelectionManager(selectionManager);
			addChildControllableContainer(easings[i]);
		}
	}
}

Array<float> AutomationKey::getValues(AutomationKey* nextKey, const float& _pos)
{
	Array<float> result;

	if (nextKey == nullptr || _pos == 0)
	{
		for (auto& v : values) result.add(v->floatValue());
		return result;
	}

	float relPos = 0;
	if(position->floatValue() < nextKey->position->floatValue()) relPos = jmap<float>(_pos, position->floatValue(), nextKey->position->floatValue(), 0, 1); 

	if (relPos < 0) relPos = 0;
	if (relPos > 1) relPos = 1;

	for (int i = 0; i < numDimensions;i++) result.add(easings[i]->getValue(values[i]->floatValue(), nextKey->values[i]->floatValue(), relPos));
	return result;
}

void AutomationKey::setValues(Array<float> newValues)
{
	for (int i = 0; i < numDimensions; i++) values[i]->setValue(newValues[i]);
}

void AutomationKey::setSelectionManager(InspectableSelectionManager * ism)
{
	BaseItem::setSelectionManager(ism);
	for(auto & e: easings) if (e != nullptr) e->setSelectionManager(ism);
}

void AutomationKey::onContainerParameterChangedInternal(Parameter * p)
{
	if (p == easingType)
	{
		setEasing((Easing::Type)(int)easingType->getValueData());
	}
}

var AutomationKey::getJSONData()
{
	var data = BaseItem::getJSONData();

	var easingsData = new DynamicObject();
	for (int i = 0; i < numDimensions; i++)
	{
		if(easings[i] != nullptr) easingsData.getDynamicObject()->setProperty("easing"+String(i), easings[i]->getJSONData());
	}

	data.getDynamicObject()->setProperty("easings",easingsData);
	return data;
}

void AutomationKey::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);

	var easingsData = data.getProperty("easings",var());
	for (int i = 0; i < numDimensions; i++)
	{
		if (easings[i] != nullptr) easings[i]->loadJSONData(data.getProperty("easing"+String(i), var()));
	}
}
