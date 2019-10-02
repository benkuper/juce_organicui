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
	viewUISize->setValue(100, 20);
	
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
