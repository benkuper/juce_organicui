/*
  ==============================================================================

    DashboardManagerUI.h
    Created: 19 Apr 2017 11:00:50pm
    Author:  Ben

  ==============================================================================
*/

#ifndef DASHBOARDMANAGERUI_H_INCLUDED
#define DASHBOARDMANAGERUI_H_INCLUDED


class DashboardManagerUI :
	public BaseManagerUI<DashboardManager, Dashboard, DashboardUI>
{
public:
	DashboardManagerUI(DashboardManager * manager);
	~DashboardManagerUI();


};


#endif  // DASHBOARDMANAGERUI_H_INCLUDED
