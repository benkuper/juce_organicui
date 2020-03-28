/*
  ==============================================================================

	AutomationRecorder.h
	Created: 12 Jan 2018 9:55:22pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

#include "../common/fitting/curve_fit_nd.h"

class AutomationRecorder :
	public EnablingControllableContainer,
	public Inspectable::InspectableListener
{
public:
	AutomationRecorder();
	~AutomationRecorder();

	TargetParameter * input;
	FloatParameter * simplificationFactor;
	BoolParameter * arm;
	BoolParameter * autoDisarm;
	EnumParameter* defaultEasing;

	WeakReference<Parameter> currentInput;

	BoolParameter * isRecording;

	Array<Point<float>> keys;

	void setCurrentInput(Parameter * input);

	void clearKeys();
	void addKeyAt(float time);

	void startRecording();
	void cancelRecording();
	Array<AutomationKey *> stopRecordingAndGetKeys();

	bool shouldRecord();

	void onContainerParameterChanged(Parameter * p) override;
	
	void inspectableDestroyed(Inspectable * i) override;

	InspectableEditor * getEditor(bool isRoot) override;

	static Array<AutomationKey *> getSimplifiedKeys(Array<Point<float>> arr, float epsilon);
	static const std::pair<int, float> findMaximumDistance(Array<Point<float>> arr);
};