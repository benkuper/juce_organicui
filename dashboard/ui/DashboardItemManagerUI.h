/*
  ==============================================================================

    DashboardItemManagerUI.h
    Created: 23 Apr 2017 12:33:58pm
    Author:  Ben

  ==============================================================================
*/

#ifndef DASHBOARDITEMMANAGERUI_H_INCLUDED
#define DASHBOARDITEMMANAGERUI_H_INCLUDED


class DashboardItemManagerUI :
	public BaseManagerUI<DashboardItemManager, DashboardItem, DashboardItemUI>
{
public:
	DashboardItemManagerUI(DashboardItemManager * manager);
	~DashboardItemManagerUI();
};



#endif  // DASHBOARDITEMMANAGERUI_H_INCLUDED
