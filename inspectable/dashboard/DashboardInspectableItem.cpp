#include "DashboardInspectableItem.h"
DashboardInspectableItem::DashboardInspectableItem(Inspectable* item) :
	DashboardItem(item),
	inspectable(nullptr),
	inspectableItemNotifier(20)
{
	if(item != nullptr) setInspectable(item);
}

DashboardInspectableItem::~DashboardInspectableItem()
{
	
}

void DashboardInspectableItem::clearItem()
{
	setInspectable(nullptr);
}

void DashboardInspectableItem::setInspectable(Inspectable* i)
{
	if (inspectable == i) return;
	
	if (!inspectable.wasObjectDeleted() && inspectable != nullptr)
	{
		inspectable->removeInspectableListener(this);
	}

	inspectable = i;

	if (inspectable != nullptr)
	{
		inspectable->addInspectableListener(this);
	}

	setInspectableInternal(inspectable);
	
	inspectableItemNotifier.addMessage(new InspectableItemEvent(InspectableItemEvent::INSPECTABLE_CHANGED, inspectable));
}

void DashboardInspectableItem::inspectableDestroyed(Inspectable* i)
{
	if (!Engine::mainEngine->isClearing && i == inspectable)
	{
		//TEMP simple solution
		NLOGWARNING("Dashboard", "Lost item reference, removing item");
		remove();
		return;
		//setInspectable(nullptr);
	}
}