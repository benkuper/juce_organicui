/*
  ==============================================================================

	Dashboard.cpp
	Created: 19 Apr 2017 10:57:04pm
	Author:  Ben

  ==============================================================================
*/


Dashboard::Dashboard() :
	BaseItem("Dashboard", false),
	isBeingEdited(false)
{
	addChildControllableContainer(&itemManager);
	itemManager.addBaseManagerListener(this);
}

Dashboard::~Dashboard()
{
}

void Dashboard::itemAdded(DashboardItem* item)
{
	item->addDashboardItemListener(this);
}

void Dashboard::itemRemoved(DashboardItem* item)
{
	item->removeDashboardItemListener(this);
}

void Dashboard::itemDataFeedback(var data)
{
	dashboardListeners.call(&DashboardListener::itemDataFeedback, data);
}

var Dashboard::getJSONData()
{
	var data = BaseItem::getJSONData();
	data.getDynamicObject()->setProperty("itemManager", itemManager.getJSONData());
	if (isBeingEdited) data.getDynamicObject()->setProperty("editing", true);
	return data;
}

void Dashboard::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	itemManager.loadJSONData(data.getProperty("itemManager", var()));
	isBeingEdited = data.getProperty("editing", false);
}

var Dashboard::getServerData()
{
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("id", shortName);
	data.getDynamicObject()->setProperty("name", niceName);
	data.getDynamicObject()->setProperty("size", itemManager.canvasSize->value);

	var iData;
	for (auto& i : itemManager.items)
	{
		iData.append(i->getServerData());
	}
	data.getDynamicObject()->setProperty("items", iData);
	return data;
}
