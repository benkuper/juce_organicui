/*
  ==============================================================================

    ParameterAutomation.h
    Created: 26 Apr 2018 3:17:25pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ParameterAutomation :
	public BaseItem,
	public Timer
{
public:
	enum Mode { LOOP, PING_PONG };
	
	ParameterAutomation(Parameter * parameter);
	virtual ~ParameterAutomation();

	Parameter* timeParamRef;
	Parameter* lengthParamRef;
	Parameter* valueParamRef;
	ControllableContainer* automationContainer;

	bool manualMode;

	bool valueIsNormalized; //for automations

	void setup();

	WeakReference<Parameter> parameter;

	EnumParameter* mode;

	void setManualMode(bool manualMode);

	virtual void setLength(float value, bool stretch = false, bool stickToEnd = false) {}
	virtual void setAllowKeysOutside(bool value) {}

	float lastUpdateTime;
	bool reversePlay; //pingPong

	virtual InspectableEditor* getContentEditor(bool isRoot);

	virtual void onContainerParameterChangedInternal(Parameter *) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void timerCallback() override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;
};

template<class T>
class ParameterNumberAutomation :
	public ParameterAutomation
{
public:
	ParameterNumberAutomation(Parameter* parameter, bool addDefaultItems = true);
	~ParameterNumberAutomation() {}

	Automation<T> automation;

	void setLength(float value, bool stretch = false, bool stickToEnd = false);
	void setAllowKeysOutside(bool value);
};

class ParameterColorAutomation :
	public ParameterAutomation
{
public:
	ParameterColorAutomation(ColorParameter* colorParam, bool addDefaultItems = true);
	~ParameterColorAutomation() {}

	void setLength(float value, bool stretch = false, bool stickToEnd = false);
	void setAllowKeysOutside(bool value);

	GradientColorManager colorManager;
	ColorParameter* colorParam;
};