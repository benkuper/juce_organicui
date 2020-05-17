#pragma once

class ParameterDetectiveWatcher :
	public ControllableDetectiveWatcher
{
public:
	ParameterDetectiveWatcher(Parameter * p = nullptr);
	~ParameterDetectiveWatcher();

	Array<var> watchedValues;

	WeakReference<Parameter> parameter;

	InspectableEditor* getEditor(bool isRoot) override;

	void onExternalParameterValueChanged(Parameter* p) override;
};