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

	ScopedPointer<InspectableEditor> automationEditor;
	ScopedPointer<ControllableUI> modeUI;
	ScopedPointer<ControllableUI> lengthUI;

	void resized() override;
};
