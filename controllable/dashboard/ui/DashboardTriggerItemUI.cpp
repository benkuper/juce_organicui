#include "DashboardTriggerItemUI.h"

DashboardTriggerItemUI::DashboardTriggerItemUI(DashboardTriggerItem* triggerItem) :
	DashboardControllableItemUI(triggerItem),
	triggerItem(triggerItem)
{
	rebuildUI();
}


DashboardTriggerItemUI::~DashboardTriggerItemUI()
{
}

ControllableUI* DashboardTriggerItemUI::createControllableUI()
{
	File f = triggerItem->customImagePath->getFile();
	if (f.existsAsFile())
	{
		Image img = ImageCache::getFromFile(f);
		return triggerItem->trigger->createImageUI(img);
	}

	return DashboardControllableItemUI::createControllableUI();
}

void DashboardTriggerItemUI::updateUIParameters()
{
	DashboardControllableItemUI::updateUIParameters();

	if (itemUI == nullptr) return;

	((TriggerUI*)itemUI.get())->useCustomBGColor = triggerItem->bgColor->enabled;
	if (triggerItem->bgColor->enabled) ((TriggerUI*)itemUI.get())->customBGColor = triggerItem->bgColor->getColor();
}

void DashboardTriggerItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	DashboardControllableItemUI::controllableFeedbackUpdateInternal(c);

	if (c == triggerItem->customImagePath) rebuildUI();
	else if (c == triggerItem->bgColor) updateUIParameters();
}

