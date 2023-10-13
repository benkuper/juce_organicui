/*
  ==============================================================================

    AutomationUI.h
    Created: 11 Dec 2016 1:22:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ParameterAutomationEditor :
	public juce::Component
{
public:
	ParameterAutomationEditor(ParameterAutomation * ppa);
	~ParameterAutomationEditor();

	ParameterAutomation * ppa;

	std::unique_ptr<InspectableEditor> automationEditor;
	std::unique_ptr<ControllableUI> modeUI;
	std::unique_ptr<ControllableUI> lengthUI;

	void resized() override;
};
