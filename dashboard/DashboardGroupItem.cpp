/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardGroupItem::DashboardGroupItem() :
	DashboardItem()
{
}

DashboardGroupItem::~DashboardGroupItem()
{
}

DashboardItemUI * DashboardGroupItem::createUI()
{
	return new DashboardGroupItemUI(this);
}
