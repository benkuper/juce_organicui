#include "DashboardItem.h"
/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardItem::DashboardItem(Inspectable * _target) :
	BaseItem("Dashboard Item",false),
	dashboardItemNotifier(5)
{
	nameCanBeChangedByUser = false;
	setTarget(_target);
}

DashboardItem::~DashboardItem()
{
	setTarget(nullptr);
}

void DashboardItem::setTarget(Inspectable * newTarget)
{
	if (target == newTarget) return;
	if (target != nullptr)
	{
		target->removeInspectableListener(this);
	}

	target = newTarget;

	if (newTarget != nullptr)
	{
		target->addInspectableListener(this);
	}

	dashboardItemNotifier.addMessage(new DashboardItemEvent(DashboardItemEvent::TARGET_CHANGED));
}

void DashboardItem::inspectableDestroyed(Inspectable * i)
{
	if (i == target.get()) setTarget(nullptr);
}

var DashboardItem::getJSONData()
{
	var data = BaseItem::getJSONData();

	if (target != nullptr && !target.wasObjectDeleted())
	{
		Controllable * c = dynamic_cast<Controllable *>(target.get());
		if (c != nullptr) data.getDynamicObject()->setProperty("target", c->getControlAddress());
	}
	else
	{
		ControllableContainer * cc = dynamic_cast<ControllableContainer *>(target.get());
		if (cc != nullptr) data.getDynamicObject()->setProperty("target", cc->getControlAddress());
	}

	return data;
}

void DashboardItem::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	if (data.hasProperty("target")) setTarget(Engine::mainEngine->getControllableForAddress(data.getProperty("target", "")));
}

DashboardItemUI * DashboardItem::createUI()
{
	return new DashboardItemUI(this);
}
