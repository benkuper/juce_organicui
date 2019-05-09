#include "DashboardItemUI.h"
/*
  ==============================================================================

    DashboardItemUI.cpp
    Created: 19 Apr 2017 11:01:13pm
    Author:  Ben

  ==============================================================================
*/


DashboardItemUI::DashboardItemUI(DashboardItem * item) :
	BaseItemUI(item, Direction::ALL)
{
	item->addAsyncDashboardItemListener(this);
	setSize(100, 100);
}

DashboardItemUI::~DashboardItemUI()
{
	if (!inspectable.wasObjectDeleted()) item->removeAsyncDashboardItemListener(this);
}

void DashboardItemUI::newMessage(const DashboardItem::DashboardItemEvent & e)
{
	switch (e.type)
	{
	case DashboardItem::DashboardItemEvent::TARGET_CHANGED:
		updateTargetUI();
		break;
	}
}
