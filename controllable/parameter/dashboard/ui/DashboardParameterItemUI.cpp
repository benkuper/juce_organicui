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

	int s = (int)parameterItem->style->getValueData();

	switch (parameterItem->parameter->type)
	{
	case Controllable::BOOL:
	{
		switch (s)
		{
		case 10:
			return new ColorStatusUI(parameterItem->parameter.get(), true);
			break;

		case 11:
			return new ColorStatusUI(parameterItem->parameter.get(), false);
			break;
		case 0:

		default:
			File f = parameterItem->btImage->getFile();
			if (f.existsAsFile())
			{
				return ((BoolParameter*)parameterItem->parameter.get())->createToggle(ImageCache::getFromFile(f));
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
			FloatSliderUI* sliderUI = ((FloatParameter*)parameterItem->parameter.get())->createSlider();
			sliderUI->orientation = s == 0 ? FloatSliderUI::HORIZONTAL : (s == 1 ? FloatSliderUI::VERTICAL : FloatSliderUI::ROTARY);
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
	break;

	case Controllable::POINT2D:
		switch (s)
		{
		case 12:
			return new P2DUI((Point2DParameter*)parameterItem->parameter.get());
			break;

		default:
			DoubleSliderUI* dbui = new DoubleSliderUI((Point2DParameter*)parameterItem->parameter.get());
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
	DashboardControllableItemUI::controllableStateUpdateInternal(c);

	if (c == parameterItem->showValue || c == parameterItem->bgColor || c == parameterItem->fgColor) updateUIParameters();
	else if (c == parameterItem->btImage || c == parameterItem->style) rebuildUI();
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
	if (b == &bt) ColorStatusUI::ColorOptionManager::show(dpi->parameter, this);
}
