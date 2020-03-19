/*
  ==============================================================================

    DashboardUI.h
    Created: 19 Apr 2017 11:06:43pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardUI :
	public BaseItemUI<Dashboard>
{
public:
	DashboardUI(Dashboard * item);
	~DashboardUI();
};