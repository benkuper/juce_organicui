#include "JuceHeader.h"

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
	if (parameterItem == nullptr
		|| parameterItem->parameter == nullptr
		|| parameterItem->parameter.wasObjectDeleted()) return nullptr;

	int s = (int)parameterItem->style->getValueData();
	Parameter* parameter = parameterItem->parameter.get();
	if (s == -1 && parameter->type == Controllable::FLOAT && parameterItem->hasCustomRange())
	{
		auto* fp = static_cast<FloatParameter*>(parameter);
		if (fp->defaultUI == FloatParameter::NONE && fp->customUI == FloatParameter::NONE) return fp->createSlider();
	}

	switch (parameter->type)
	{
	case Controllable::BOOL:
	{
		switch (s)
		{
		case 10:
			return new ColorStatusUI(parameter, true);
			break;

		case 11:
			return new ColorStatusUI(parameter, false);
			break;

		case 23:
		case 25:
		{

			BoolToggleUI* bp = ((BoolParameter*)parameter)->createButtonToggle();
			if (s == 25) bp->momentaryMode = true;
			return bp;
		}
		break;

		case 0:
		case 22:
		case 24:
		default:
		{
			File f = parameterItem->btImage->getFile();
			BoolToggleUI* bp = ((BoolParameter*)parameter)->createToggle(ImageCache::getFromFile(f));
			if (s == 24) bp->momentaryMode = true;
			return bp;
		}
		break;
		}
	}
	break;

	case Controllable::FLOAT:
	case Controllable::INT:
	case Controllable::ENUM:
	{
		switch (s)
		{
		case 0:
		case 1:
		case 5:
		{
			FloatSliderUI* sliderUI = ((FloatParameter*)parameter)->createSlider();
			sliderUI->orientation = s == 0 ? FloatSliderUI::HORIZONTAL : (s == 1 ? FloatSliderUI::VERTICAL : FloatSliderUI::ROTARY);
			return sliderUI;
		}
		break;

		case 2:
			return ((FloatParameter*)parameter)->createLabelParameter();
			break;

		case 3:
			return ((FloatParameter*)parameter)->createTimeLabelParameter();
			break;



		case 10:
			return new ColorStatusUI(parameter, true);
			break;

		case 11:
			return new ColorStatusUI(parameter, false);
			break;


		case 20:
		{
			EnumParameterButtonBarUI* ui = ((EnumParameter*)parameter)->createButtonBarUI();
			ui->isVertical = false;
			return ui;
		}
		break;


		case 21:
		{
			EnumParameterButtonBarUI* ui = ((EnumParameter*)parameter)->createButtonBarUI();
			ui->isVertical = true;
			return ui;
		}
		break;
		}
		break;
	}
	break;

	case Controllable::POINT2D:
		switch (s)
		{
		case 12:
			return new P2DUI((Point2DParameter*)parameter);
			break;

		default:
			DoubleSliderUI* dbui = new DoubleSliderUI((Point2DParameter*)parameter);
			dbui->canShowExtendedEditor = false;
			return dbui;
			break;
		}
		break;

	default:
		break;

	}

	return DashboardControllableItemUI::createControllableUI();
}

void DashboardParameterItemUI::updateUIParametersInternal()
{
	if (itemUI == nullptr || parameterItem == nullptr
		|| parameterItem->parameter == nullptr
		|| parameterItem->parameter.wasObjectDeleted()
		|| parameterItem->inspectable.wasObjectDeleted()) return;

	if (ParameterUI* pui = dynamic_cast<ParameterUI*>(itemUI.get()))
	{
		if (parameterItem->hasCustomRange()) pui->setCustomRange(parameterItem->getRangeBound(false), parameterItem->getRangeBound(true));
		else pui->clearCustomRange();
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

		pui->customTextSize = parameterItem->textSize->enabled ? parameterItem->textSize->intValue() : -1;

		if (TargetParameterUI* tpui = dynamic_cast<TargetParameterUI*>(pui))
		{
			if (DashboardTargetParameterItem* tpItem = dynamic_cast<DashboardTargetParameterItem*>(parameterItem))
			{
				tpui->useCustomShowFullAddressInEditor = tpItem->showFullAddress->enabled;
				if (tpItem->showFullAddress->enabled) tpui->customShowFullAddressInEditor = tpItem->showFullAddress->boolValue();

				tpui->useCustomShowParentNameInEditor = tpItem->showParentName->enabled;
				if (tpItem->showParentName->enabled) tpui->customShowParentNameInEditor = tpItem->showParentName->boolValue();

				tpui->customParentLabelSearch = tpItem->parentLabelLevel->enabled ? tpItem->parentLabelLevel->intValue() : -1;

				tpui->useCustomShowLearnButton = tpItem->showLearnButton->enabled;
				if (tpItem->showLearnButton->enabled) tpui->customShowLearnButton = tpItem->showLearnButton->boolValue();
			}
		}
	}
}

void DashboardParameterItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	DashboardControllableItemUI::controllableFeedbackUpdateInternal(c);
	if (c == parameterItem->useCustomRange)
	{
		rebuildUI();
		return;
	}

	if (c == parameterItem->showValue || c == parameterItem->bgColor || c == parameterItem->fgColor
		|| c == parameterItem->useCustomRange || c == parameterItem->customRange
		|| c == parameterItem->customRangeY || c == parameterItem->customRangeZ) updateUIParameters();
	else if (c == parameterItem->btImage || c == parameterItem->style) rebuildUI();
	else if (DashboardTargetParameterItem* tpItem = dynamic_cast<DashboardTargetParameterItem*>(parameterItem))
	{
		if (c == tpItem->showFullAddress || c == tpItem->showParentName || c == tpItem->parentLabelLevel || c == tpItem->showLearnButton) updateUIParameters();
	}
}

void DashboardParameterItemUI::controllableStateUpdateInternal(Controllable* c)
{
	DashboardControllableItemUI::controllableStateUpdateInternal(c);

	if (c == parameterItem->showValue || c == parameterItem->bgColor || c == parameterItem->fgColor
		|| c == parameterItem->useCustomRange || c == parameterItem->customRange
		|| c == parameterItem->customRangeY || c == parameterItem->customRangeZ) updateUIParameters();
	else if (c == parameterItem->btImage || c == parameterItem->style) rebuildUI();
	else if (DashboardTargetParameterItem* tpItem = dynamic_cast<DashboardTargetParameterItem*>(parameterItem))
	{
		if (c == tpItem->showFullAddress || c == tpItem->showParentName || c == tpItem->parentLabelLevel || c == tpItem->showLearnButton) updateUIParameters();
	}
}

DashboardParameterStyleEditor::DashboardParameterStyleEditor(Parameter* p, DashboardParameterItem* dpi, bool isRoot) :
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
	if (b == &bt)
	{
		auto modifiers = ModifierKeys::getCurrentModifiers();
		if (modifiers.isRightButtonDown() || modifiers.isCommandDown() )
		{
			PopupMenu m;
			m.addItem(1, "Copy Style");
			m.addItem(2, "Paste Style");
			m.showMenuAsync(PopupMenu::Options(), [&](int result)
				{

					if (result == 1)
					{
						var cMapData;
						HashMap<var, Colour>::Iterator it(dpi->parameter->colorStatusMap);
						while (it.next())
						{
							cMapData.append(it.getKey());
							var colorVar;
							Colour c = it.getValue();
							colorVar.append(c.getFloatRed());
							colorVar.append(c.getFloatGreen());
							colorVar.append(c.getFloatBlue());
							colorVar.append(c.getFloatAlpha());
							cMapData.append(colorVar);
						}
						SystemClipboard::copyTextToClipboard(JSON::toString(cMapData));
					}
					else if (result == 2)
					{
						var cMapData = JSON::parse(SystemClipboard::getTextFromClipboard());
						for (int i = 0; i < cMapData.size(); i += 2)
						{
							var cData = cMapData[i + 1];
							Colour c = Colour::fromFloatRGBA((float)cData[0], (float)cData[1], (float)cData[2], (float)cData[3]);
							dpi->parameter->colorStatusMap.set(cMapData[i], c);
						}
					}
				});
		}
		ColorStatusUI::ColorOptionManager::show(dpi->parameter, this);
	}
}
