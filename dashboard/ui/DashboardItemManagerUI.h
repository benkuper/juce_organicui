/*
  ==============================================================================

    DashboardItemManagerUI.h
    Created: 23 Apr 2017 12:33:58pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemManagerUI :
	public BaseManagerViewUI<DashboardItemManager, DashboardItem, DashboardItemUI>
{
public:
	DashboardItemManagerUI(DashboardItemManager * manager);
	~DashboardItemManagerUI();

	DashboardItemUI * createUIForItem(DashboardItem *) override;
};