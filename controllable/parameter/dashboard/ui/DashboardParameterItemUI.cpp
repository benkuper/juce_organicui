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
			return ((BoolParameter*)parameterItem->parameter.get())->createToggle(ImageCache::getFromFile(f));
		}
	}
	break;

	case Controllable::FLOAT:
	case Controllable::INT:
	case Controllable::ENUM:
	{
		switch ((int)parameterItem->style->getValueData())
		{
		case 0:
		case 1:
		{
			FloatSliderUI* sliderUI = ((FloatParameter*)parameterItem->parameter.get())->createSlider();
			sliderUI->orientation = (int)parameterItem->style->getValueData() == 0 ? FloatSliderUI::HORIZONTAL : FloatSliderUI::VERTICAL;
			return sliderUI;
		}
			break;

		case 2:
			return ((FloatParameter*)parameterItem->parameter.get())->createLabelParameter();
			break;

		case 3:
			return ((FloatParameter*)parameterItem->parameter.get())->createTimeLabelParameter();
			break;

		case 10:
			return new ColorStatusUI(parameterItem->parameter.get(), true);
			break;

		case 11:
			return new ColorStatusUI(parameterItem->parameter.get(), false);
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

void DashboardParameterItemUI::controllableStateUpdateInternal(Controllable* c)
{
	if (c == parameterItem->showValue || c == parameterItem->bgColor || c == parameterItem->fgColor) updateUIParameters();
	else if (c == parameterItem->btImage || c == parameterItem->style) rebuildUI();
}

DashboardParameterStyleEditor::DashboardParameterStyleEditor(Parameter* p, DashboardParameterItem * dpi, bool isRoot) :
	ParameterEditor(p, isRoot),
	dpi(dpi),
	bt("Edit...")
{
	bt.addListener(this);
	addAndMakeVisible(&bt);
}

DashboardParameterStyleEditor::~DashboardParameterStyleEditor()
{
}

void DashboardParameterStyleEditor::resizedInternal(juce::Rectangle<int>& r)
{
	bt.setBounds(r.removeFromRight(100));
	r.removeFromRight(4);
	ParameterEditor::resizedInternal(r);
}

void DashboardParameterStyleEditor::buttonClicked(Button* b)
{
	ParameterEditor::buttonClicked(b);
	if (b == &bt) ColorStatusUI::ColorOptionManager::show(dpi->parameter, this);
}
