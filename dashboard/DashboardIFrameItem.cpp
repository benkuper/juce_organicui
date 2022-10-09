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
	showWarningInUI = true;
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

var DashboardIFrameItem::getItemParameterFeedback(Parameter* p)
{
	var data = DashboardItem::getItemParameterFeedback(p);
	data.getDynamicObject()->setProperty("targetType", this->getTypeString());

	return data;
}


DashboardItemUI* DashboardIFrameItem::createUI()
{
	return new DashboardIFrameItemUI(this);
}
