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
};