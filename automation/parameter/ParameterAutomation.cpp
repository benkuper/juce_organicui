#include "ParameterAutomation.h"
/*
  ==============================================================================

	ParameterAutomation.cpp
	Created: 26 Apr 2018 3:17:25pm
	Author:  Ben

  ==============================================================================
*/


ParameterAutomation::ParameterAutomation(Parameter* _parameter) :
	BaseItem(_parameter->niceName +" automation", false),
	parameter(_parameter),
	automationContainer(automationContainer),
	timeParamRef(timeParamRef),
	lengthParamRef(lengthParamRef),
	mode(nullptr),
	manualMode(true)
{
	isSelectable = false;
	parameter->setControllableFeedbackOnly(true);
}

ParameterAutomation::~ParameterAutomation()
{
	if (!parameter.wasObjectDeleted() && parameter != nullptr) parameter->setControllableFeedbackOnly(false);
}

void ParameterAutomation::setup()
{
	automationContainer->editorCanBeCollapsed = false;
	automationContainer->editorIsCollapsed = false;
	automationContainer->isSelectable = false;
	addChildControllableContainer(automationContainer);
}

void ParameterAutomation::setManualMode(bool value)
{
	if (value == manualMode) return;

	manualMode = value;

	if (manualMode)
	{
		stopTimer();

		if (mode != nullptr)
		{
			removeControllable(mode);
			mode = nullptr;
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

		startTimerHz(50);
		
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
		parameter->setValue(valueParamRef->value);
	}
}


void ParameterAutomation::timerCallback()
{
	if (mode == nullptr)
	{
		stopTimer();
		return;
	}

	float t = Time::getMillisecondCounter() / 1000.0f;
	float delta = t - lastUpdateTime;

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

	lastUpdateTime = t;
}

var ParameterAutomation::getJSONData()
{
	var data = BaseItem::getJSONData();
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
	timeParamRef = automation.position;
	lengthParamRef = automation.length;
	valueParamRef = automation.value;
	automationContainer = &automation;
	
	setup();

	automation.enableSnap->setValue(false);
	automation.showUIInEditor = true;

	if (addDefaultItems)
	{
		automation.addItem(0, parameter->getNormalizedValue(), false);
		automation.items[0]->setEasing(Easing::BEZIER);
		automation.addItem(automation.length->floatValue(), parameter->getNormalizedValue(), false);
	}

}

void ParameterNumberAutomation::setLength(float value, bool stretch, bool stickToEnd)
{
	automation.setLength(value, stretch, stickToEnd);
}

void ParameterNumberAutomation::setAllowKeysOutside(bool value)
{
	automation.allowKeysOutside = true;
}



ParameterColorAutomation::ParameterColorAutomation(ColorParameter* colorParam, bool addDefaultItems) :
	ParameterAutomation(colorParam),
	colorParam(colorParam),
	colorManager(1,addDefaultItems)
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
