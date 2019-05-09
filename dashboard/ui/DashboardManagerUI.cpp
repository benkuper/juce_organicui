/*
  ==============================================================================

	DashboardManagerUI.cpp
	Created: 19 Apr 2017 11:00:50pm
	Author:  Ben

  ==============================================================================
*/


DashboardManagerUI::DashboardManagerUI(DashboardManager * manager) :
	BaseManagerUI("Dashboards", manager)
{
	bgColor = bgColor.darker();
	addExistingItems(false);
}

DashboardManagerUI::~DashboardManagerUI()
{

}
