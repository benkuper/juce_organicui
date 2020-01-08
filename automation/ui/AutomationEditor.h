/*
  ==============================================================================

    AutomationEditor.h
    Created: 5 Feb 2017 2:58:27pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationTimelineEditor :
	public GenericControllableContainerEditor
{
public:
	AutomationTimelineEditor(AutomationBase * automation, bool isRoot);
	~AutomationTimelineEditor();

	AutomationBase * automation;
	std::unique_ptr<AutomationTimelineUIBase> automationUI;

	void resizedInternalContent(juce::Rectangle<int> &r) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationTimelineEditor)
};
