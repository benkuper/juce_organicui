#include "DashboardItem.h"
/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardItem::DashboardItem(Inspectable * _target) :
	BaseItem("Dashboard Item",false)
{
	viewUISize->setPoint(100, 20);
	
	viewUIPosition->hideInEditor = false;
	viewUISize->hideInEditor = false;
}

DashboardItem::~DashboardItem()
{
}

DashboardItemUI* DashboardItem::createUI()
{
	return new DashboardItemUI(this);
}

var DashboardItem::getServerData()
{
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("position", viewUIPosition->value);
	data.getDynamicObject()->setProperty("size", viewUISize->value);
	return data;
}

void DashboardItem::notifyDataFeedback(var data)
{
	dashboardItemsListeners.call(&DashboardItemListener::itemDataFeedback, data);
}
