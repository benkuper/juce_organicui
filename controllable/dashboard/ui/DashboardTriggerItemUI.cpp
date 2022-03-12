
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
		return triggerItem->trigger->createImageUI(img, triggerItem->keepSaturation->boolValue());
	}

	return DashboardControllableItemUI::createControllableUI();
}

void DashboardTriggerItemUI::updateUIParametersInternal()
{
	if (itemUI == nullptr) return;
	
	TriggerUI* tui = (TriggerUI*)itemUI.get();

	tui->useCustomBGColor = triggerItem->bgColor->enabled;
	if (triggerItem->bgColor->enabled) tui->customBGColor = triggerItem->bgColor->getColor();

	tui->useCustomTextColor = triggerItem->textColor->enabled;
	if (triggerItem->textColor->enabled) tui->customTextColor = triggerItem->textColor->getColor();

	tui->customTextSize = triggerItem->textSize->enabled ? triggerItem->textSize->intValue() : -1;
}

void DashboardTriggerItemUI::updateEditModeInternal(bool editMode)
{
	DashboardControllableItemUI::updateEditModeInternal(editMode);
}

void DashboardTriggerItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	DashboardControllableItemUI::controllableFeedbackUpdateInternal(c);

	if (c == triggerItem->customImagePath || c == triggerItem->keepSaturation) rebuildUI();
	else if (c == triggerItem->bgColor) updateUIParameters();
}

