#include "JuceHeader.h"

DashboardInspectableItem::DashboardInspectableItem(Inspectable* item) :
	DashboardItem(item),
	inspectable(nullptr),
	settingInspectable(false),
	inspectableItemNotifier(20)
{
	target = addTargetParameter("Target", "The target for this dashboard item");
}

DashboardInspectableItem::~DashboardInspectableItem()
{
}

void DashboardInspectableItem::clearItem()
{
	BaseItem::clearItem();

	setInspectable(nullptr);
	Engine::mainEngine->removeControllableContainerListener(this);
}

void DashboardInspectableItem::setInspectable(Inspectable* i)
{
	if (inspectable == i) return;

	settingInspectable = true;

	if (!inspectable.wasObjectDeleted() && inspectable != nullptr)
	{
		inspectable->removeInspectableListener(this);
	}

	inspectable = i;

	if (inspectable != nullptr)
	{
		ghostInspectable();
		inspectable->addInspectableListener(this);
		Engine::mainEngine->removeControllableContainerListener(this);
	}

	setInspectableInternal(inspectable);

	inspectableItemNotifier.addMessage(new InspectableItemEvent(InspectableItemEvent::INSPECTABLE_CHANGED, inspectable));

	settingInspectable = false;
}

void DashboardInspectableItem::onContainerParameterChangedInternal(Parameter* p)
{
	DashboardItem::onContainerParameterChangedInternal(p);
	if (p == target)
	{
		if (!isCurrentlyLoadingData && !settingInspectable)
		{
			setInspectable(target->target.get());
		}
	}

}

void DashboardInspectableItem::inspectableDestroyed(Inspectable* i)
{
	DashboardItem::inspectableDestroyed(i);

	if (!Engine::mainEngine->isClearing && i == inspectable)
	{
		Engine::mainEngine->addControllableContainerListener(this);
		setInspectable(nullptr);
	}
}

void DashboardInspectableItem::childStructureChanged(ControllableContainer* cc)
{
	if (cc == Engine::mainEngine)
	{
		if (inspectable.wasObjectDeleted() || inspectable == nullptr)
		{
			checkGhost();
		}

		return;
	}

	DashboardItem::childStructureChanged(cc);
}


var DashboardInspectableItem::getJSONData(bool includeNonOverriden)
{
	var data = DashboardItem::getJSONData(includeNonOverriden);
	if (inspectableGhostAddress.isNotEmpty()) data.getDynamicObject()->setProperty("ghostAddress", inspectableGhostAddress);
	return data;
}

void DashboardInspectableItem::loadJSONDataItemInternal(var data)
{
	DashboardItem::loadJSONDataItemInternal(data);

	inspectableGhostAddress = data.getProperty("ghostAddress", inspectableGhostAddress);
	if (inspectable.wasObjectDeleted() || inspectable == nullptr)
	{
		Engine::mainEngine->addControllableContainerListener(this);
	}
}
