/*
  ==============================================================================

	AutomationRecorder.h
	Created: 12 Jan 2018 9:55:22pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationKey;

class AutomationRecorder :
	public EnablingControllableContainer,
	public Inspectable::InspectableListener
{
public:
	AutomationRecorder();
	~AutomationRecorder();

	TargetParameter * input;
	BoolParameter* normalize;
	BoolParameter * arm;
	BoolParameter * autoDisarm;

	enum SimplificationMethod { SIMPL_BEZIER, SIMPL_LINEAR, SIMPL_LINEAR_INTERACTIVE };
	EnumParameter* simplificationMethod;
	FloatParameter* simplificationTolerance;

	WeakReference<Parameter> currentInput;

	BoolParameter * isRecording;

	struct RecordValue
	{
		RecordValue(float t = 0, var v = var()) : time(t), value(v) {}
		float time;
		var value;
	};

	Array<RecordValue> keys;

	void setCurrentInput(Parameter * input);

	void clearKeys();
	void removeKeysAfter(float time);
	void addKeyAt(float time);

	void startRecording();
	void cancelRecording();
	Array<RecordValue> stopRecordingAndGetKeys();

	bool shouldRecord();

	void onContainerParameterChanged(Parameter * p) override;
	
	void inspectableDestroyed(Inspectable * i) override;

	class  RecorderEvent
	{
	public:
		enum Type { RECORDER_UPDATED };

		RecorderEvent(Type t) : type(t) {}
		Type type;
	};

	QueuedNotifier<RecorderEvent> recorderNotifier;
	typedef QueuedNotifier<RecorderEvent>::Listener AsyncListener;

	void addAsyncRecorderListener(AsyncListener* newListener) { recorderNotifier.addListener(newListener); }
	void addAsyncCoalescedRecorderListener(AsyncListener* newListener) { recorderNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncRecorderListener(AsyncListener* listener) { recorderNotifier.removeListener(listener); }

	InspectableEditor * getEditorInternal(bool isRoot) override;
};

