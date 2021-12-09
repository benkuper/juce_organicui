/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardItem::DashboardItem(Inspectable * _target, const String &name) :
	BaseItem(name, false)
{
	viewUIPosition->hideInEditor = false;
	viewUISize->hideInEditor = false;
	isUILocked->hideInEditor = false;

	viewUISize->setPoint(400, 300);
}

DashboardItem::~DashboardItem()
{
}

DashboardItemUI* DashboardItem::createUI()
{
	return new DashboardItemUI(this);
}

var DashboardItem::getServerData()
{
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("name", shortName);
	data.getDynamicObject()->setProperty("itemControlAddress", getControlAddress(DashboardManager::getInstance()));
	data.getDynamicObject()->setProperty("type", getTypeString());
	data.getDynamicObject()->setProperty("position", viewUIPosition->value);
	data.getDynamicObject()->setProperty("size", viewUISize->value);
	return data;
}

void DashboardItem::onContainerParameterChangedInternal(Parameter* p)
{
	notifyDataFeedback(getItemParameterFeedback(p));
}

var DashboardItem::getItemParameterFeedback(Parameter* p)
{
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("feedbackType", "uiFeedback");
	data.getDynamicObject()->setProperty("controlAddress", p->getControlAddress(DashboardManager::getInstance()));
	data.getDynamicObject()->setProperty("type", p->getTypeString());
	data.getDynamicObject()->setProperty("value", p->value);
	return data;
}

void DashboardItem::notifyDataFeedback(var data)
{
	dashboardItemsListeners.call(&DashboardItemListener::itemDataFeedback, data);
}
