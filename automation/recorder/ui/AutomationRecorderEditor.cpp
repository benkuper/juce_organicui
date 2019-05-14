/*
  ==============================================================================

	AutomationRecorderEditor.cpp
	Created: 06 May 2019 8:11:42pm
	Author:  bkupe

  ==============================================================================
*/

AutomationRecorderEditor::AutomationRecorderEditor(AutomationRecorder * recorder, bool isRoot) :
	GenericControllableContainerEditor(recorder, isRoot),
	recorder(recorder)
{
	updateSourceUI();
}

AutomationRecorderEditor::~AutomationRecorderEditor()
{
}

void AutomationRecorderEditor::updateSourceUI()
{
	if (sourceFeedbackUI != nullptr) removeChildComponent(sourceFeedbackUI);
	if (recorder->currentInput != nullptr)
	{
		sourceFeedbackUI = recorder->currentInput->createDefaultUI();
		//sourceFeedbackUI->setForceFeedbackOnly(true);
		addAndMakeVisible(sourceFeedbackUI);
	}

	resized();
}

void AutomationRecorderEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (sourceFeedbackUI != nullptr) sourceFeedbackUI->setBounds(r.removeFromRight(jmin(r.getWidth() - 150, 200)).reduced(2));
	GenericControllableContainerEditor::resizedInternalHeader(r);
}

void AutomationRecorderEditor::controllableFeedbackUpdate(Controllable * c)
{
	updateSourceUI();
}

