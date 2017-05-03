/*
  ==============================================================================

    Dashboard.h
    Created: 19 Apr 2017 10:57:04pm
    Author:  Ben

  ==============================================================================
*/

#ifndef DASHBOARD_H_INCLUDED
#define DASHBOARD_H_INCLUDED

class Dashboard :
	public BaseItem
{
public:
	Dashboard();
	~Dashboard();

	DashboardPanelManager panelManager;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

};



#endif  // DASHBOARD_H_INCLUDED
