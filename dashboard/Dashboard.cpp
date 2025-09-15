/*
  ==============================================================================

	Dashboard.cpp
	Created: 19 Apr 2017 10:57:04pm
	Author:  Ben

  ==============================================================================
*/


Dashboard::Dashboard() :
	BaseItem("Dashboard", false),
	isBeingEdited(false),
	dashboardNotifier(5)
{
	//itemManager.editorIsCollapsed = true;
	password = addStringParameter("Password", "Password for web clients for this specific dashboard, leave empty public access", "");
	unlockOnce = addBoolParameter("Unlock Only Once", "If checked, this will allow to only have to unlock once per session. Refreshing the page will reset the lock.", false);
	addChildControllableContainer(&itemManager);
	itemManager.addManagerListener(this);
	itemManager.addDashboardFeedbackListener(this);
}

Dashboard::~Dashboard()
{
	if(!itemManager.isBeingDestroyed) itemManager.removeDashboardFeedbackListener(this);
}

void Dashboard::setIsBeingEdited(bool value)
{
	if (value == isBeingEdited) return;
	isBeingEdited = value;
	
	dashboardNotifier.addMessage(new DashboardEvent(DashboardEvent::EDITING_UPDATE, this));
}

void Dashboard::itemAdded(BaseItem* item)
{
	if (Engine::mainEngine->isClearing || isClearing) return;
	((DashboardItem*)item)->addDashboardFeedbackListener(this);
	dashboardListeners.call(&DashboardListener::askForRefresh, this);

}

void Dashboard::itemsAdded(Array<BaseItem*> items)
{
	if (Engine::mainEngine->isClearing || isClearing) return;
	for (auto& i : items) ((DashboardItem*)i)->addDashboardFeedbackListener(this);
	dashboardListeners.call(&DashboardListener::askForRefresh, this);
}

void Dashboard::itemRemoved(BaseItem* item)
{
	if (Engine::mainEngine->isClearing || isClearing) return;
	((DashboardItem*)item)->removeDashboardFeedbackListener(this);
	dashboardListeners.call(&DashboardListener::askForRefresh, this);
}

void Dashboard::itemsRemoved(Array<BaseItem*> items)
{
	if (Engine::mainEngine->isClearing || isClearing) return;
	for (auto& i : items)
	{
		if (i->isClearing || i->isBeingDestroyed) continue;
		((DashboardItem*)i)->removeDashboardFeedbackListener(this);
	}
	dashboardListeners.call(&DashboardListener::askForRefresh, this);
}

void Dashboard::parameterFeedback(var data)
{
	if (Engine::mainEngine->isClearing || isClearing) return;
	dashboardListeners.call(&DashboardListener::parameterFeedback, data);
}

void Dashboard::dashboardFeedback(var data)
{
	if (Engine::mainEngine->isClearing || isClearing) return;
	dashboardListeners.call(&DashboardListener::dashboardFeedback, data);
}

var Dashboard::getJSONData(bool includeNonOverriden)
{
	var data = BaseItem::getJSONData(includeNonOverriden);
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
	data.getDynamicObject()->setProperty("bgColor", itemManager.bgColor->value);

	if (itemManager.bgImage->stringValue().isNotEmpty())
	{
		data.getDynamicObject()->setProperty("bgImage", itemManager.bgImage->getControlAddress());
		data.getDynamicObject()->setProperty("bgImageScale", itemManager.bgImageScale->getValue());
		data.getDynamicObject()->setProperty("bgImageAlpha", itemManager.bgImageAlpha->getValue());
	}

	if (password->stringValue().isNotEmpty())
	{
		data.getDynamicObject()->setProperty("password", password->stringValue());
		data.getDynamicObject()->setProperty("unlockOnce", unlockOnce->boolValue());
	}

	itemManager.fillServerData(data);
	return data;
}
