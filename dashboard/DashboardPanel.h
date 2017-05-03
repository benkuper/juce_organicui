/*
  ==============================================================================

    DashboardPanel.h
    Created: 23 Apr 2017 12:20:05pm
    Author:  Ben

  ==============================================================================
*/

#ifndef DASHBOARDPANEL_H_INCLUDED
#define DASHBOARDPANEL_H_INCLUDED

class DashboardPanel :
	public BaseItem
{
public:
	DashboardPanel();
	~DashboardPanel();
	

	DashboardItemManager itemManager;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;
};



#endif  // DASHBOARDPANEL_H_INCLUDED
