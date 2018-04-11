/*
  ==============================================================================

    DashboardPanelUI.h
    Created: 23 Apr 2017 12:20:30pm
    Author:  Ben

  ==============================================================================
*/

#ifndef DASHBOARDPANELUI_H_INCLUDED
#define DASHBOARDPANELUI_H_INCLUDED


class DashboardPanelUI :
	public BaseItemUI<DashboardPanel>
{
public:
	DashboardPanelUI(DashboardPanel * item);
	~DashboardPanelUI();

	DashboardItemManagerUI managerUI;

	void resizedInternalContent(juce::Rectangle<int> &r) override;
};



#endif  // DASHBOARDPANELUI_H_INCLUDED
