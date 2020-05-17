#pragma once

class ParameterDetectiveWatcher :
	public ControllableDetectiveWatcher
{
public:
	ParameterDetectiveWatcher(Parameter * p = nullptr);
	~ParameterDetectiveWatcher();

	WeakReference<Parameter> parameter;

	void onExternalParameterValueChanged(Parameter* p) override;

	virtual ControllableDetectiveWatcherUI* getUI();
};