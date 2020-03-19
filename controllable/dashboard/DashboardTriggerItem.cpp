#include "DashboardTriggerItem.h"
DashboardTriggerItem::DashboardTriggerItem(Trigger * item) :
	DashboardControllableItem(item),
	trigger(item)
{
	bgColor = addColorParameter("Background Color", "Color of the background", BG_COLOR.brighter(), false);
	bgColor->canBeDisabledByUser = true;
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
