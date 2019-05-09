#include "DashboardItemManagerUI.h"
/*
  ==============================================================================

    DashboardItemManagerUI.cpp
    Created: 23 Apr 2017 12:33:58pm
    Author:  Ben

  ==============================================================================
*/

DashboardItemManagerUI::DashboardItemManagerUI(DashboardItemManager * manager) :
	BaseManagerViewUI("Dashboard", manager)
{
	//bgColor = Colours::purple;
	//setWantsKeyboardFocus(true);

	addExistingItems(false);
}

DashboardItemManagerUI::~DashboardItemManagerUI()
{
}

DashboardItemUI * DashboardItemManagerUI::createUIForItem(DashboardItem * item)
{
	return item->createUI();
}
