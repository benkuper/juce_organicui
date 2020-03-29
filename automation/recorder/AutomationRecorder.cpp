/*
  ==============================================================================

	AutomationRecorder.cpp
	Created: 12 Jan 2018 9:55:22pm
	Author:  Ben

  ==============================================================================
*/

AutomationRecorder::AutomationRecorder() :
	EnablingControllableContainer("Recorder")
{
	input = addTargetParameter("Input Value", "Input value used for recording");
	input->showTriggers = false;
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

void AutomationRecorder::addKeyAt(float time)
{
	if (isRecording->boolValue() && currentInput != nullptr)
	{
		keys.add(Point<float>(time, currentInput->hasRange() ? currentInput->getNormalizedValue() : jlimit<float>(0, 1, currentInput->getValue())));
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
}

void AutomationRecorder::cancelRecording()
{
	isRecording->setValue(false);
	clearKeys();
}

Array<Point<float>> AutomationRecorder::stopRecordingAndGetKeys()
{
	isRecording->setValue(false);
	if (autoDisarm->boolValue()) arm->setValue(false);

	Array<Point<float>> result;
	result.addArray(keys);

	clearKeys();

	return result;
}

bool AutomationRecorder::shouldRecord()
{
	return input->target != nullptr && arm->boolValue();
}

void AutomationRecorder::onContainerParameterChanged(Parameter * p)
{
	if (p == input) setCurrentInput(dynamic_cast<Parameter *>(input->target.get()));
}

void AutomationRecorder::inspectableDestroyed(Inspectable * i)
{
	if (!currentInput.wasObjectDeleted() && i == currentInput) setCurrentInput(nullptr);
}

InspectableEditor * AutomationRecorder::getEditor(bool isRoot)
{
	return new AutomationRecorderEditor(this, isRoot);
}