/*
  ==============================================================================

	ParameterAutomation.cpp
	Created: 26 Apr 2018 3:17:25pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

ParameterAutomation::ParameterAutomation(Parameter* _parameter) :
	BaseItem(_parameter->niceName + " automation", false),
	Thread("ParameterAutomation"),
	timeParamRef(nullptr),
	lengthParamRef(nullptr),
	valueParamRef(nullptr),
	automationContainer(nullptr),
	manualMode(true),
	valueIsNormalized(false),
	parameter(_parameter),
	mode(nullptr)
{
	isSelectable = false;
	parameter->setControllableFeedbackOnly(true);
	setManualMode(false);

}

ParameterAutomation::~ParameterAutomation()
{
	stopThread(1000);
	if (!parameter.wasObjectDeleted() && parameter != nullptr) parameter->setControllableFeedbackOnly(false);
}

void ParameterAutomation::setup()
{
	automationContainer->editorCanBeCollapsed = false;
	automationContainer->editorIsCollapsed = false;
	automationContainer->isSelectable = false;
	addChildControllableContainer(automationContainer);

	startThread();
}

void ParameterAutomation::setManualMode(bool value)
{
	if (value == manualMode) return;

	manualMode = value;

	if (manualMode)
	{

		if (mode != nullptr)
		{
			removeControllable(mode);
			mode = nullptr;
			stopThread(1000);
		}
	}
	else
	{
		if (mode == nullptr)
		{
			//Must call setup from child classes
			mode = addEnumParameter("Play Mode", "Play mode");
			mode->addOption("Loop", LOOP)->addOption("Ping Pong", PING_PONG);
		}

		startThread();
	}
}

InspectableEditor* ParameterAutomation::getContentEditor(bool isRoot)
{
	return automationContainer->getEditor(isRoot);
}

void ParameterAutomation::onContainerParameterChangedInternal(Parameter* p)
{
	BaseItem::onContainerParameterChangedInternal(p);
	if (p == enabled)
	{
		if (parameter != nullptr && !parameter.wasObjectDeleted()) parameter->setControllableFeedbackOnly(enabled->boolValue());
	}
}

void ParameterAutomation::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	BaseItem::onControllableFeedbackUpdateInternal(cc, c);
	if (c == lengthParamRef)
	{
		timeParamRef->setRange(0, lengthParamRef->floatValue());
	}
	else if (c == valueParamRef)
	{
		if (valueIsNormalized) parameter->setNormalizedValue(valueParamRef->value);
		else parameter->setValue(valueParamRef->value);
	}
}

void ParameterAutomation::run()
{
	if (mode == nullptr || timeParamRef == nullptr || lengthParamRef == nullptr) return;

	double lastUpdateTime = Time::getMillisecondCounter() / 1000.0;

	while (!threadShouldExit())
	{
		double t = Time::getMillisecondCounter() / 1000.0;
		double delta = t - lastUpdateTime;

		if (!manualMode)
		{
			Mode m = mode->getValueDataAsEnum<Mode>();
			if (m == LOOP) timeParamRef->setValue(fmodf(timeParamRef->floatValue() + delta, lengthParamRef->floatValue()));
			else if (m == PING_PONG)
			{
				float ft = timeParamRef->floatValue() + delta * (reversePlay ? -1 : 1);
				if (ft < 0 || ft > lengthParamRef->floatValue())
				{
					reversePlay = !reversePlay;
					ft = timeParamRef->floatValue() + delta * (reversePlay ? -1 : 1);
				}

				timeParamRef->setValue(ft);
			}
		}

		lastUpdateTime = Time::getMillisecondCounter() / 1000.0;
		wait(20); //50fps
	}
}

var ParameterAutomation::getJSONData(bool includeNonOverriden)
{
	var data = BaseItem::getJSONData(includeNonOverriden);
	data.getDynamicObject()->setProperty("automation", automationContainer->getJSONData());
	return data;
}

void ParameterAutomation::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	automationContainer->loadJSONData(data.getProperty("automation", var()));
}

// NUMBER


ParameterNumberAutomation::ParameterNumberAutomation(Parameter* parameter, bool addDefaultItems) :
	ParameterAutomation(parameter)
{
	length = addFloatParameter("Length", "Length of this parameter animation", automation.length->floatValue(), .1f);
	length->defaultUI = FloatParameter::TIME;
	length->hideInEditor = true;
	timeParamRef = automation.position;
	lengthParamRef = length;
	valueParamRef = automation.value;
	automationContainer = &automation;

	valueIsNormalized = false;

	setup();

	//automation.enableSnap->setValue(false);
	if (parameter->hasRange()) automation.valueRange->setPoint(parameter->minimumValue, parameter->maximumValue);
	else automation.valueRange->setEnabled(false);

	if (addDefaultItems)
	{
		AutomationKey* k = automation.addKey(0, parameter->floatValue(), false);
		k->setEasing(Easing::BEZIER);
		automation.addKey(automation.length->floatValue(), parameter->floatValue(), false);
	}

}

void ParameterNumberAutomation::setLength(float value, bool stretch, bool stickToEnd)
{
	automation.setLength(value, stretch, stickToEnd);
	length->setValue(value);
}

void ParameterNumberAutomation::setAllowKeysOutside(bool value)
{
	//automation.allowKeysOutside = true;
}

void ParameterNumberAutomation::onContainerParameterChangedInternal(Parameter* p)
{
	ParameterAutomation::onContainerParameterChangedInternal(p);
	if (p == length) setLength(length->floatValue(), true);
}

ParameterColorAutomation::ParameterColorAutomation(ColorParameter* colorParam, bool addDefaultItems) :
	ParameterAutomation(colorParam),
	colorManager(1, addDefaultItems),
	colorParam(colorParam)
{
	timeParamRef = colorManager.position;
	lengthParamRef = colorManager.length;
	valueParamRef = colorManager.currentColor;
	automationContainer = &colorManager;

	setup();
}

void ParameterColorAutomation::setLength(float value, bool stretch, bool stickToEnd)
{
	colorManager.setLength(value, stretch, stickToEnd);
}

void ParameterColorAutomation::setAllowKeysOutside(bool value)
{
	colorManager.allowKeysOutside = true;
}
