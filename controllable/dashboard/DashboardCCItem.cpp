#include "DashboardCCItem.h"
DashboardCCItem::DashboardCCItem(ControllableContainer* container) :
	DashboardInspectableItem(container),
	container(container)
{
	ghostInspectable();
}

DashboardCCItem::~DashboardCCItem()
{

}

DashboardItemUI* DashboardCCItem::createUI()
{
	return new DashboardCCItemUI(this);
}
void DashboardCCItem::ghostInspectable()
{
	if (!inspectable.wasObjectDeleted() && inspectable != nullptr) inspectableGhostAddress = dynamic_cast<ControllableContainer *>(inspectable.get())->getControlAddress();
}

void DashboardCCItem::checkGhost()
{
	setInspectable(Engine::mainEngine->getControllableContainerForAddress(inspectableGhostAddress));
}

var DashboardCCItem::getJSONData()
{
	var data = DashboardInspectableItem::getJSONData();
	if(container != nullptr) data.getDynamicObject()->setProperty("container", container->getControlAddress());
	return data;
}

void DashboardCCItem::loadJSONDataItemInternal(var data)
{

	String address = data.getProperty("container", inspectableGhostAddress);
	setInspectable(Engine::mainEngine->getControllableContainerForAddress(address));

	DashboardInspectableItem::loadJSONDataItemInternal(data);
}

void DashboardCCItem::setInspectableInternal(Inspectable* i)
{
	container = dynamic_cast<ControllableContainer*>(i);
}
