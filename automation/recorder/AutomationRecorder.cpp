#include "AutomationRecorder.h"
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

	//input->customGetTargetFunc = &ModuleManager::showAllValuesAndGetControllable;
	//input->customGetControllableLabelFunc = &Module::getTargetLabelForValueControllable;
	//input->customCheckAssignOnNextChangeFunc = &ModuleManager::checkControllableIsAValue;

	arm = addBoolParameter("Arm", "If set, when a sequence will play, this will start recording. In any case, when a sequence is stopped or seeked, the recording stops as well", false);
	//arm->setEnabled(input->target != nullptr);
	//arm->enabled = input->target != nullptr;
	autoDisarm = addBoolParameter("Auto Disarm", "If set, the arm parameter will be automatically set to off when a record has been saved", false);

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
		keys.add(RecordValue(time, currentInput->getValue()));
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
}

void AutomationRecorder::inspectableDestroyed(Inspectable * i)
{
	if (!currentInput.wasObjectDeleted() && i == currentInput) setCurrentInput(nullptr);
}

InspectableEditor * AutomationRecorder::getEditor(bool isRoot)
{
	return new AutomationRecorderEditor(this, isRoot);
}