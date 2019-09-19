#include "DashboardManagerUI.h"
/*
  ==============================================================================

	DashboardManagerUI.cpp
	Created: 19 Apr 2017 11:00:50pm
	Author:  Ben

  ==============================================================================
*/


DashboardManagerUI::DashboardManagerUI(DashboardManager* manager) :
	BaseManagerUI("Dashboards", manager)
{
	setDefaultLayout(Layout::HORIZONTAL);

	bgColor = bgColor.darker();
	addExistingItems(false);

	editModeUI.reset(manager->editMode->createToggle());
	addAndMakeVisible(editModeUI.get());
	editModeUI->showLabel = true;
}


DashboardManagerUI::~DashboardManagerUI()
{

}

void DashboardManagerUI::resizedInternalContent(Rectangle<int>& r)
{
	editModeUI->setBounds(r.removeFromRight(100).reduced(10));
	BaseManagerUI::resizedInternalContent(r);
}