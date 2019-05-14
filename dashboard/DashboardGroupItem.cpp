/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardGroupItem::DashboardGroupItem(Inspectable * _target) :
	BaseItem("Dashboard Group Item",false)
{
}

DashboardGroupItem::~DashboardGroupItem()
{
}

BaseItemMinimalUI<DashboardItem> * DashboardGroupItem::createUI()
{
	return nullptr;// new DashboardItemUI(this);
}
