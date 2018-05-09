#include "ParameterAutomation.h"
/*
  ==============================================================================

	ParameterAutomation.cpp
	Created: 26 Apr 2018 3:17:25pm
	Author:  Ben

  ==============================================================================
*/

ParameterAutomation::ParameterAutomation(ControllableContainer * rootContainer) :
	BaseItem("ParamAutomation"),
	parameter(nullptr)
{
	target = addTargetParameter("Target", "The parameter to automate", rootContainer);
	ParameterAutomation((Parameter *)nullptr);
}

ParameterAutomation::ParameterAutomation(Parameter * _parameter) :
	BaseItem(_parameter->niceName),
	target(nullptr)
{
	setParameter(_parameter);

	automation.isSelectable = false;
	isSelectable = false;

	addChildControllableContainer(&automation);
	automation.enableSnap->setValue(false);
	automation.editorCanBeCollapsed = false;
	automation.editorIsCollapsed = false;
	automation.showUIInEditor = true;
}

ParameterAutomation::~ParameterAutomation()
{
	setParameter(nullptr);
}

void ParameterAutomation::setParameter(Parameter * p)
{
	if (parameter == p) return;
	if (parameter != nullptr && !parameter.wasObjectDeleted())
	{
		parameter->setControllableFeedbackOnly(false);
	}

	parameter = p;

	if (parameter != nullptr)
	{
		setNiceName(parameter->niceName);
		automation.setNiceName("Automation for " + parameter->niceName);
		parameter->setControllableFeedbackOnly(enabled->boolValue());
		if (automation.items.size() == 0)
		{
			automation.addItem(0, parameter->getNormalizedValue(), false);
			automation.items[0]->setEasing(Easing::BEZIER);
			automation.addItem(automation.length->floatValue(), parameter->getNormalizedValue(), false);

		}
	}
}

void ParameterAutomation::onContainerParameterChangedInternal(Parameter * p)
{
	BaseItem::onContainerParameterChangedInternal(p);
	if (p == enabled)
	{
		if (parameter != nullptr && !parameter.wasObjectDeleted()) parameter->setControllableFeedbackOnly(enabled->boolValue());
	} else if (p == target)
	{
		setParameter(dynamic_cast<Parameter *>(target->target.get()));
	}
}

void ParameterAutomation::onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c)
{
	BaseItem::onControllableFeedbackUpdateInternal(cc, c);
	if (c == automation.value && parameter != nullptr) parameter->setNormalizedValue(automation.value->floatValue());
}

var ParameterAutomation::getJSONData()
{
	var data = BaseItem::getJSONData();
	data.getDynamicObject()->setProperty("automation", automation.getJSONData());
	return data;
}

void ParameterAutomation::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	automation.loadJSONData(data.getProperty("automation", var()));
}



//Playable

PlayableParameterAutomation::PlayableParameterAutomation(Parameter * parameter) :
	ParameterAutomation(parameter)
{
	mode = addEnumParameter("Play Mode", "Play mode");

	currentTime = addFloatParameter("Time", "Current time if manual", 0, 0, automation.length->floatValue());

	mode->addOption("Loop", LOOP)->addOption("Ping Pong", PING_PONG)->addOption("Manual", MANUAL);
	startTimerHz(50);

	mode->hideInEditor = mode->getValueDataAsEnum<Mode>() == MANUAL;
}

PlayableParameterAutomation::PlayableParameterAutomation()
{
}

void PlayableParameterAutomation::onContainerParameterChangedInternal(Parameter * p)
{
	ParameterAutomation::onContainerParameterChangedInternal(p);

	if (p == mode)
	{
		Mode m = mode->getValueDataAsEnum<Mode>();
		mode->hideInEditor = m == MANUAL;
		if (m == MANUAL) stopTimer();
		else startTimerHz(50);

		lastUpdateTime = Time::getMillisecondCounter() / 1000.0f;
	} else if (p == currentTime)
	{
		automation.position->setValue(currentTime->floatValue());
	}
}

void PlayableParameterAutomation::onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c)
{
	ParameterAutomation::onControllableFeedbackUpdateInternal(cc, c);
	if (c == automation.length)
	{
		currentTime->setRange(0, automation.length->floatValue());
	}
}


void PlayableParameterAutomation::timerCallback()
{
	float t = Time::getMillisecondCounter() / 1000.0f;
	float delta = t - lastUpdateTime;

	Mode m = mode->getValueDataAsEnum<Mode>();
	if (m == LOOP) currentTime->setValue(fmodf(currentTime->floatValue() + delta, automation.length->floatValue()));
	else if (m == PING_PONG)
	{
		float ft = currentTime->floatValue() + delta * (reversePlay ? -1 : 1);
		if (ft < 0 || ft > automation.length->floatValue())
		{
			reversePlay = !reversePlay;
			ft = currentTime->floatValue() + delta * (reversePlay ? -1 : 1);
		}

		currentTime->setValue(ft);
	}

	lastUpdateTime = t;
}
