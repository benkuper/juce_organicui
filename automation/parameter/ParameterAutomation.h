/*
  ==============================================================================

    ParameterAutomation.h
    Created: 26 Apr 2018 3:17:25pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ParameterAutomation :
	public BaseItem
{
public:
	ParameterAutomation(ControllableContainer * rootContainer = nullptr);
	ParameterAutomation(Parameter * parameter);
	virtual ~ParameterAutomation();

	Automation automation;
	TargetParameter * target;
	
	WeakReference<Parameter> parameter;
	void setParameter(Parameter * p);

	virtual void onContainerParameterChangedInternal(Parameter *) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) override;

	virtual var getJSONData() override;
	virtual void loadJSONDataInternal(var data) override;
};


class PlayableParameterAutomation :
	public ParameterAutomation,
	public Timer
{
public:
	PlayableParameterAutomation(Parameter * parameter);
	PlayableParameterAutomation();

	enum Mode {LOOP, PING_PONG, MANUAL };

	EnumParameter * mode;
	FloatParameter * currentTime;

	float lastUpdateTime;
	bool reversePlay; //pingPong

	virtual void onContainerParameterChangedInternal(Parameter *) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) override;

	void timerCallback() override;
};