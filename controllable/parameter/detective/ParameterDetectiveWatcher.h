#pragma once

class ParameterDetectiveWatcher :
	public ControllableDetectiveWatcher
{
public:
	ParameterDetectiveWatcher();
	virtual ~ParameterDetectiveWatcher();

	juce::WeakReference<Parameter> parameter;

	virtual void setControllable(Controllable* c) override;

	void onExternalParameterValueChanged(Parameter* p) override;

	DECLARE_UI_FUNC;
};
