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
	viewUISize->setBounds(20, 20,2000,2000);
	viewUISize->setValue(100, 20);
}

DashboardItem::~DashboardItem()
{
}

DashboardItemUI* DashboardItem::createUI()
{
	return new DashboardItemUI(this);
}
