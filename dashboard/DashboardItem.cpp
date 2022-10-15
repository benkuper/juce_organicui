/*
  ==============================================================================

	DashboardItem.cpp
	Created: 19 Apr 2017 11:06:51pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

DashboardItem::DashboardItem(Inspectable* _target, const String& name) :
	BaseItem(name, false)
{
	useCustomArrowKeysBehaviour = true;

	hideInEditor = true;
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
	notifyParameterFeedback(p);
}

void DashboardItem::onControllableStateChanged(Controllable* c)
{
	if (Parameter* p = dynamic_cast<Parameter*>(c)) notifyParameterFeedback(p);
}
