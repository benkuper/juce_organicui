#include "DashboardControllableItem.h"
/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardControllableItem::DashboardControllableItem(Controllable * controllable) :
	DashboardItem(controllable)
{
	updateName();
}

DashboardControllableItem::~DashboardControllableItem()
{
}

void DashboardControllableItem::setTarget(Inspectable * newTarget)
{
	
	DashboardItem::setTarget(newTarget);
	updateName();
}

void DashboardControllableItem::updateName()
{
	if (target != nullptr && !target.wasObjectDeleted())
	{
		Controllable * c = (Controllable *)target.get();
		setNiceName((c->parentContainer != nullptr ? c->parentContainer->niceName + " : " : "") + c->niceName);
	}
}

DashboardItemUI * DashboardControllableItem::createUI()
{
	return new DashboardControllableUI(this);
}
