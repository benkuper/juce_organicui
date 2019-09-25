#include "DashboardParameterItemUI.h"
DashboardParameterItemUI::DashboardParameterItemUI(DashboardParameterItem* item) :
	DashboardControllableItemUI(item),
	parameterItem(item)
{
	rebuildUI();
	if (itemUI != nullptr) setSize(item->viewUISize->x, item->viewUISize->y);
}

DashboardParameterItemUI::~DashboardParameterItemUI()
{
}

ControllableUI* DashboardParameterItemUI::createControllableUI()
{
	if (parameterItem->parameter->type == Controllable::BOOL)
	{
		File f = parameterItem->btImage->getFile();
		if (f.existsAsFile())
		{
			ImageButton * b = AssetManager::getInstance()->getToggleBTImage(ImageCache::getFromFile(f));
			return ((BoolParameter*)parameterItem->parameter)->createImageToggle(b);
		}
	}

	return DashboardControllableItemUI::createControllableUI();
}

void DashboardParameterItemUI::updateUIParameters()
{
	DashboardControllableItemUI::updateUIParameters();

	if (parameterItem == nullptr || parameterItem->inspectable.wasObjectDeleted()) return;

	ParameterUI* pui = dynamic_cast<ParameterUI*>(itemUI.get());
	if (pui != nullptr)
	{
		pui->showValue = parameterItem->showValue->boolValue();

		if (parameterItem->bgColor != nullptr)
		{
			pui->useCustomBGColor = parameterItem->bgColor->enabled;
			pui->customBGColor = parameterItem->bgColor->getColor();
		}

		if (parameterItem->fgColor != nullptr)
		{
			pui->useCustomFGColor = parameterItem->fgColor->enabled;
			pui->customFGColor = parameterItem->fgColor->getColor();
		}
	}
}

void DashboardParameterItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	DashboardControllableItemUI::controllableFeedbackUpdateInternal(c);

	if (c == parameterItem->showValue || c == parameterItem->bgColor || c == parameterItem->fgColor) updateUIParameters();
	else if (c == parameterItem->btImage) rebuildUI();
}
