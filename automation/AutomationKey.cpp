/*
  ==============================================================================

	AutomationKey.cpp
	Created: 11 Dec 2016 1:22:20pm
	Author:  Ben

  ==============================================================================
*/

AutomationKey::AutomationKey(float minimumValue, float maximumValue) :
	BaseItem("Key",false)
{
	itemDataType = "AutomationKey";
	hideEditorHeader = true;

	position = addFloatParameter("Position", "Position of the key", 0);
	position->defaultUI = FloatParameter::TIME;
	value = addFloatParameter("Value", "Value of the key", 0, minimumValue, maximumValue);

	easingType = addEnumParameter("EasingType", "Type of transition to the next key");
	for (int i = 0; i < Easing::TYPE_MAX; i++) easingType->addOption(Easing::typeNames[i], (Easing::Type)i, false);

	easingType->setValueWithData(Easing::BEZIER);
	easingType->hideInEditor = true;
	easingType->forceSaveValue = true;

	canInspectChildContainers = false;


	helpID = "AutomationKey";
}


AutomationKey::~AutomationKey()
{
}

void AutomationKey::setEasing(Easing::Type t)
{

	if (easing != nullptr)
	{
		if (easing->type == t) return;
		removeChildControllableContainer(easing.get());
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

	default:
		break;
	}

	easing.reset(e);

	if (easing != nullptr)
	{
        easing->setSelectionManager(selectionManager);
		addChildControllableContainer(easing.get());
	}
}

float AutomationKey::getValue(AutomationKey * nextKey, const float & _pos)
{
	if (position == nullptr || nextKey == nullptr) return 0;

	float relPos = 0;
	if(position->floatValue() < nextKey->position->floatValue()) relPos = jmap<float>(_pos, position->floatValue(), nextKey->position->floatValue(), 0, 1); 

	if (relPos < 0) relPos = 0;
	if (relPos > 1) relPos = 1;

	return easing->getValue(value->floatValue(), nextKey->value->floatValue(), relPos);
}

void AutomationKey::setSelectionManager(InspectableSelectionManager * ism)
{
	BaseItem::setSelectionManager(ism);
	if (easing != nullptr) easing->setSelectionManager(ism);
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
	if (easing != nullptr) data.getDynamicObject()->setProperty("easing", easing->getJSONData());
	return data;
}

void AutomationKey::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	if (easing != nullptr) easing->loadJSONData(data.getProperty("easing", var()));
}
