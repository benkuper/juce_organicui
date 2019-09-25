#include "DashboardTriggerItem.h"
DashboardTriggerItem::DashboardTriggerItem(Trigger * item) :
	DashboardControllableItem(item),
	trigger(item)
{
	customImagePath = addFileParameter("Custom Image", "Choose a custom image instead of the default button");
	customImagePath->fileTypeFilter = "*.png;*.jpg;*.jpeg;*.gif";
}

DashboardTriggerItem::~DashboardTriggerItem()
{

}

DashboardItemUI* DashboardTriggerItem::createUI()
{
	return new DashboardTriggerItemUI(this);
}

void DashboardTriggerItem::setInspectableInternal(Inspectable* i)
{
	DashboardControllableItem::setInspectableInternal(i);
	trigger = dynamic_cast<Trigger*>(i);
}
