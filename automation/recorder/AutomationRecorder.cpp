/*
  ==============================================================================

	AutomationRecorder.cpp
	Created: 12 Jan 2018 9:55:22pm
	Author:  Ben

  ==============================================================================
*/

AutomationRecorder::AutomationRecorder() :
	EnablingControllableContainer("Recorder"),
	recorderNotifier(2)
{
	input = addTargetParameter("Input Value", "Input value used for recording");
	input->excludeTypesFilter.add(Trigger::getTypeStringStatic());

	normalize = addBoolParameter("Normalize", "If checked, this will normalize the input value to a 0-1 range. Only works if the input has a range", false, false);

	arm = addBoolParameter("Arm", "If set, when a sequence will play, this will start recording. In any case, when a sequence is stopped or seeked, the recording stops as well", false);
	autoDisarm = addBoolParameter("Auto Disarm", "If set, the arm parameter will be automatically set to off when a record has been saved", false);
	
	simplificationMethod = addEnumParameter("Simplification Method", "This decides which algorithm to use when simplifying the recorded data.\nBezier is using the least square approach and is best suited for natural curves.\nLinear is using the Douglas-Peucker method, and works best for straight lines and linear signal.\nLinear Interactive allows you to change the simplification interactively after the recording, where Linear will apply it automatically (useful for automated recording).");
	simplificationMethod->addOption("Bezier (Least square)", SIMPL_BEZIER)->addOption("Linear (Douglas-Peucker)", SIMPL_LINEAR)->addOption("Linear Interactive", SIMPL_LINEAR_INTERACTIVE);

	simplificationTolerance = addFloatParameter("Simplification Tolerance", "This is the tolerance of the simplification to apply to the recorded data. A higher value means more points will remain. Only available for Linear method right now.", .2f, 0, 1, false);

	

	isRecording = addBoolParameter("Is Recording", "Is the recorder currently recording or eating pasta", false);
	isRecording->isControllableFeedbackOnly = true;
}

AutomationRecorder::~AutomationRecorder()
{
	setCurrentInput(nullptr);
}

void AutomationRecorder::setCurrentInput(Parameter * newInput)
{
	if (!currentInput.wasObjectDeleted() && currentInput != nullptr)
	{
		//currentInput->removeParameterListener(this);
		currentInput->removeInspectableListener(this);
	}

	currentInput = newInput;

	if (!currentInput.wasObjectDeleted() && currentInput != nullptr)
	{
		//currentInput->addParameterListener(this);
		currentInput->addInspectableListener(this);
		normalize->setEnabled(currentInput->hasRange());
		//arm->setEnabled(input->target != nullptr);
	}
}

void AutomationRecorder::clearKeys()
{
	keys.clear();
}

void AutomationRecorder::removeKeysAfter(float time)
{
	int amountToRemove = 0;
	for (int i = keys.size() - 1; i >= 0; i--)
	{
		if (keys[i].time >= time) amountToRemove++;
		else break;
	}

	if(amountToRemove > 0) keys.removeLast(amountToRemove);
}

void AutomationRecorder::addKeyAt(float time)
{
	if (isRecording->boolValue() && currentInput != nullptr)
	{
		var val = (normalize->enabled && normalize->boolValue()) ? currentInput->getNormalizedValue() : currentInput->getValue();
		keys.add(RecordValue(time, val));
		recorderNotifier.addMessage(new RecorderEvent(RecorderEvent::RECORDER_UPDATED));
	}
}

void AutomationRecorder::startRecording()
{
	if (currentInput == nullptr)
	{
		LOG("Can't record when input is not set !");
		return;
	}

	if (isRecording->boolValue())
	{
		DBG("Already recording, cancel before start again");
		cancelRecording();
	}

	isRecording->setValue(true);

	recorderNotifier.addMessage(new RecorderEvent(RecorderEvent::RECORDER_UPDATED));
}

void AutomationRecorder::cancelRecording()
{
	isRecording->setValue(false);
	clearKeys();

	recorderNotifier.addMessage(new RecorderEvent(RecorderEvent::RECORDER_UPDATED));
}

Array<AutomationRecorder::RecordValue> AutomationRecorder::stopRecordingAndGetKeys()
{
	isRecording->setValue(false);
	if (autoDisarm->boolValue()) arm->setValue(false);

	Array<RecordValue> result;
	result.addArray(keys);

	clearKeys();

	recorderNotifier.addMessage(new RecorderEvent(RecorderEvent::RECORDER_UPDATED));

	return result;
}


bool AutomationRecorder::shouldRecord()
{
	return input->target != nullptr && arm->boolValue();
}

void AutomationRecorder::onContainerParameterChanged(Parameter * p)
{
	if (p == input) setCurrentInput(dynamic_cast<Parameter *>(input->target.get()));
	else if (p == arm)
	{
		recorderNotifier.addMessage(new RecorderEvent(RecorderEvent::RECORDER_UPDATED));
	}
	else if (p == simplificationMethod)
	{
		simplificationTolerance->setEnabled(simplificationMethod->getValueDataAsEnum<SimplificationMethod>() != SIMPL_BEZIER);
	}
}

void AutomationRecorder::inspectableDestroyed(Inspectable * i)
{
	if (!currentInput.wasObjectDeleted() && i == currentInput) setCurrentInput(nullptr);
}

InspectableEditor * AutomationRecorder::getEditor(bool isRoot)
{
	return new AutomationRecorderEditor(this, isRoot);
}