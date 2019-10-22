#include "DashboardParameterItemUI.h"
DashboardParameterItemUI::DashboardParameterItemUI(DashboardParameterItem* item) :
	DashboardControllableItemUI(item),
	parameterItem(item)
{
	rebuildUI();
}

DashboardParameterItemUI::~DashboardParameterItemUI()
{
}

ControllableUI* DashboardParameterItemUI::createControllableUI()
{
	switch (parameterItem->parameter->type)
	{
	case Controllable::BOOL:
	{
		File f = parameterItem->btImage->getFile();
		if (f.existsAsFile())
		{
			ImageButton* b = AssetManager::getInstance()->getToggleBTImage(ImageCache::getFromFile(f));
			return ((BoolParameter*)parameterItem->parameter)->createImageToggle(b);
		}
	}
	break;

	case Controllable::FLOAT:
	{
		switch ((int)parameterItem->style->getValueData())
		{
		case 0:
		case 1:
		{
			FloatSliderUI* sliderUI = ((FloatParameter*)parameterItem->parameter)->createSlider();
			sliderUI->orientation = (int)parameterItem->style->getValueData() == 0 ? FloatSliderUI::HORIZONTAL : FloatSliderUI::VERTICAL;
			return sliderUI;
		}
			break;

		case 2:
			return ((FloatParameter*)parameterItem->parameter)->createLabelParameter();
			break;

		case 3:
			return ((FloatParameter*)parameterItem->parameter)->createTimeLabelParameter();
			break;
		}
	}
    
        default:
            break;
            
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
	else if (c == parameterItem->btImage || c == parameterItem->style) rebuildUI();
}
