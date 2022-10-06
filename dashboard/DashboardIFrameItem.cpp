/*
  ==============================================================================

	SharedTextureDashboardItem.cpp
	Created: 5 Dec 2021 3:53:20pm
	Author:  bkupe

  ==============================================================================
*/

DashboardIFrameItem::DashboardIFrameItem(var params) :
	DashboardItem(this, getTypeString())
{
	url = addStringParameter("URL", "URL of the iFrame to show", "https://pointerpointer.com/");
}

DashboardIFrameItem::~DashboardIFrameItem()
{

}

var DashboardIFrameItem::getServerData()
{
	var data = DashboardItem::getServerData();
	data.getDynamicObject()->setProperty("url", url->stringValue());
	return data;
}


DashboardItemUI* DashboardIFrameItem::createUI()
{
	return new DashboardIFrameItemUI(this);
}
