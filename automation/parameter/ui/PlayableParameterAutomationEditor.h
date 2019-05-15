/*
  ==============================================================================

    AutomationUI.h
    Created: 11 Dec 2016 1:22:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class PlayableParameterAutomationEditor :
	public Component
{
public:
	PlayableParameterAutomationEditor(PlayableParameterAutomation * ppa);
	~PlayableParameterAutomationEditor();

	PlayableParameterAutomation * ppa;

	std::unique_ptr<InspectableEditor> automationEditor;
	std::unique_ptr<ControllableUI> modeUI;
	std::unique_ptr<ControllableUI> lengthUI;

	void resized() override;
};
