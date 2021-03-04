/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardGroupItem::DashboardGroupItem() :
	DashboardItem(nullptr, "Group")
{
	borderWidth = addFloatParameter("Border Width", "Width of the border to show", 1, 0);
	borderColor = addColorParameter("Border Color", "Color of the border", NORMAL_COLOR);
	backgroundColor = addColorParameter("Background Color", "Color of the background", NORMAL_COLOR.withAlpha(.05f));
	backgroundColor->canBeDisabledByUser = true;

	itemManager.editorIsCollapsed = true;
    addChildControllableContainer(&itemManager);
}

DashboardGroupItem::~DashboardGroupItem()
{
}

var DashboardGroupItem::getServerData()
{
	var data = DashboardItem::getServerData();
	if(backgroundColor->enabled) data.getDynamicObject()->setProperty("backgroundColor", backgroundColor->value);
	data.getDynamicObject()->setProperty("borderWidth", borderWidth->value);
	data.getDynamicObject()->setProperty("borderColor", borderColor->value);



	return data;
}


var DashboardGroupItem::getJSONData()
{
	var data = DashboardItem::getJSONData();
	data.getDynamicObject()->setProperty("itemManager", itemManager.getJSONData());
	return data;
}

void DashboardGroupItem::loadJSONDataItemInternal(var data)
{
	DashboardItem::loadJSONDataItemInternal(data);
	itemManager.loadJSONData(data.getProperty("itemManager", var()));
}

bool DashboardGroupItem::paste()
{
	itemManager.addItemsFromClipboard();
	return true;
}

DashboardItemUI * DashboardGroupItem::createUI()
{
	return new DashboardGroupItemUI(this);
}
