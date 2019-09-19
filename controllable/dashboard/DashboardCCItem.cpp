#include "DashboardCCItem.h"
DashboardCCItem::DashboardCCItem(ControllableContainer* container) :
	DashboardInspectableItem(container),
	container(container)
{
	
}

DashboardCCItem::~DashboardCCItem()
{

}

DashboardItemUI* DashboardCCItem::createUI()
{
	return new DashboardCCItemUI(this);
}

var DashboardCCItem::getJSONData()
{
	if (container == nullptr) return var(); //not saving for the moment

	var data = DashboardItem::getJSONData();
	data.getDynamicObject()->setProperty("container", container->getControlAddress());
	return data;
}

void DashboardCCItem::loadJSONDataItemInternal(var data)
{
	DashboardItem::loadJSONDataItemInternal(data);

	String address = data.getProperty("container", "");

	setInspectable(Engine::mainEngine->getControllableContainerForAddress(address));
	if (container == nullptr)
	{
		inspectableGhostAddress = address;
		NLOGWARNING("Dashboard", "Item not found when loading : " << address << ", will not be saved");
	}

}
void DashboardCCItem::setInspectableInternal(Inspectable* i)
{
	container = dynamic_cast<ControllableContainer*>(i);
}
