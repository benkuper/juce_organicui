/*
  ==============================================================================

    DashboardManagerUI.h
    Created: 19 Apr 2017 11:00:50pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardManagerUI :
	public BaseManagerUI<DashboardManager, Dashboard, DashboardUI>
{
public:
	DashboardManagerUI(DashboardManager * manager);
	~DashboardManagerUI();

	std::unique_ptr<BoolToggleUI> editModeUI;

	void resizedInternalContent(Rectangle<int>& r) override;
};