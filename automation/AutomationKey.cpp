/*
  ==============================================================================

	AutomationKeyBase.cpp
	Created: 11 Dec 2016 1:22:20pm
	Author:  Ben

  ==============================================================================
*/

AutomationKeyBase::AutomationKeyBase() :
	BaseItem("Key",false),
	numDimensions(0)
{
	itemDataType = "AutomationKeyBase";
	hideEditorHeader = true;

	position = addFloatParameter("Position", "Position of the key", 0);
	position->defaultUI = FloatParameter::TIME;

	easingType = addEnumParameter("EasingType", "Type of transition to the next key");

	easingType->addOption("Linear", EasingBase::LINEAR);
	easingType->addOption("Bezier", EasingBase::BEZIER);
	easingType->addOption("Hold", EasingBase::HOLD);
	easingType->addOption("Sine", EasingBase::SINE);

	easingType->setValueWithData(EasingBase::LINEAR);
	easingType->hideInEditor = true;
	easingType->forceSaveValue = true;

	canInspectChildContainers = false;

	setEasing(EasingBase::LINEAR);

	helpID = "AutomationKeyBase";
}


AutomationKeyBase::~AutomationKeyBase()
{
}


float AutomationKeyBase::getRelativePosition(float pos1, float pos2, float weight) const
{
	return juce::jlimit<float>(jmap<float>(jmin(pos1, pos2), jmax(pos1, pos2), weight), 0, 1);
}


void AutomationKeyBase::setSelectionManager(InspectableSelectionManager * ism)
{
	BaseItem::setSelectionManager(ism);
	if (easingBase != nullptr) easingBase->setSelectionManager(ism);
}

void AutomationKeyBase::onContainerParameterChangedInternal(Parameter * p)
{
	if (p == easingType)
	{
		setEasing((EasingBase::Type)(int)easingType->getValueData());
	}
}

var AutomationKeyBase::getJSONData()
{
	var data = BaseItem::getJSONData();
	if (easingBase != nullptr) data.getDynamicObject()->setProperty("easing", easingBase->getJSONData());
	return data;
}

void AutomationKeyBase::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	if (easingBase != nullptr) easingBase->loadJSONData(data.getProperty("easing", var()));
}
