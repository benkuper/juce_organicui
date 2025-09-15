/*
  ==============================================================================

    DashboardManagerUI.h
    Created: 19 Apr 2017 11:00:50pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardManagerUI :
	public ManagerUI<DashboardManager, Dashboard>
{
public:
	DashboardManagerUI(DashboardManager * manager);
	~DashboardManagerUI();

	std::unique_ptr<BoolToggleUI> editModeUI;
	std::unique_ptr<BoolToggleUI> snappingUI;

	void resizedInternalContent(juce::Rectangle<int>& r) override;
};