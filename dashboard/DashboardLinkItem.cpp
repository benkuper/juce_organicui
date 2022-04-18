/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardLinkItem::DashboardLinkItem() :
	DashboardTriggerItem()
{
	target->hideInEditor = true;
	dashboardTarget = addTargetParameter("Dashboard Target", "The dashboard to go to", DashboardManager::getInstance());
	dashboardTarget->targetType = TargetParameter::CONTAINER;
	dashboardTarget->maxDefaultSearchLevel = 0;

	setInClients = addBoolParameter("Set in Clients", "If checked, this will notify any connected client to switch to the same dashboard", false);
	launchTrigger = addTrigger("Go", "When triggered, this will go to the selected dashboard");

	setInspectable(launchTrigger);
}

DashboardLinkItem::~DashboardLinkItem()
{
}

void DashboardLinkItem::onContainerTriggerTriggered(Trigger* t)
{
	DashboardTriggerItem::onContainerTriggerTriggered(t);
	if (t == launchTrigger)
	{
		if (Dashboard* d = dynamic_cast<Dashboard*>(dashboardTarget->targetContainer.get()))
		{
			DashboardManager::getInstance()->setCurrentDashboard(d, setInClients->boolValue());
		}
		
	}
}

var DashboardLinkItem::getServerData()
{
	var data = DashboardTriggerItem::getServerData();

	String targetName;
	if (Dashboard* d = dynamic_cast<Dashboard*>(dashboardTarget->targetContainer.get())) targetName = d->shortName;
	data.getDynamicObject()->setProperty("target", targetName);
	data.getDynamicObject()->setProperty("setInClients", setInClients->boolValue());

	return data;
}

DashboardItemUI* DashboardLinkItem::createUI()
{
	return new DashboardTriggerItemUI(this);
}
