/*
  ==============================================================================

    DashboardView.cpp
    Created: 19 Apr 2017 10:57:12pm
    Author:  Ben

  ==============================================================================
*/


DashboardView::DashboardView(DashboardPanelManager * manager) :
	BaseManagerViewUI("Dashboard",manager)
{
	addItemText = "Add Item";

	addExistingItems(true);
}

DashboardView::~DashboardView()
{
}