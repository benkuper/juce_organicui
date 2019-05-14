/*
  ==============================================================================

    AutomationRecorderEditor.h
    Created: 06 May 2019 8:11:42pm
    Author:  bkupe

  ==============================================================================
*/

class AutomationRecorderEditor :
	public GenericControllableContainerEditor
{
public:
	AutomationRecorderEditor(AutomationRecorder * recorder, bool isRoot);
	~AutomationRecorderEditor();

	AutomationRecorder * recorder;

	ScopedPointer<ControllableUI> sourceFeedbackUI;

	void updateSourceUI();
	void resizedInternalHeader(juce::Rectangle<int> &r) override;

	void controllableFeedbackUpdate(Controllable * c) override;
};
