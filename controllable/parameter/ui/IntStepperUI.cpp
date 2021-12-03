/*
  ==============================================================================

	IntStepperUI.cpp
	Created: 8 Mar 2016 3:46:43pm
	Author:  bkupe

  ==============================================================================
*/


bool IntStepperUI::showHexModeOption = true;


IntStepperUI::IntStepperUI(Array<Parameter*> parameters) :
	FloatStepperUI(parameters),
	intParams(Inspectable::getArrayAs<Parameter, IntParameter>(parameters)),
	intParam(dynamic_cast<IntParameter*>(parameters[0]))
{
	jassert(intParam != nullptr);

	if (intParam != nullptr && intParam->hexMode)
	{
		slider->valueFromTextFunction = &IntStepperUI::textToHexValue;
		slider->textFromValueFunction = &IntStepperUI::hexValueToText;
		slider->updateText();
	}

	slider->setNumDecimalPlacesToDisplay(0);

}

IntStepperUI::~IntStepperUI()
{
}

String IntStepperUI::hexValueToText(double v)
{
	return "0x" + String::toHexString((int)v).toUpperCase();
}

double IntStepperUI::textToHexValue(String t)
{
	return t.getHexValue32();
}

void IntStepperUI::addPopupMenuItemsInternal(PopupMenu* p)
{
	FloatStepperUI::addPopupMenuItemsInternal(p);
	if (showHexModeOption) p->addItem(-100, "Hex Mode", true, intParam->hexMode);
}

void IntStepperUI::handleMenuSelectedID(int result)
{
	FloatStepperUI::handleMenuSelectedID(result);
	if (result == -100)
	{
		intParam->hexMode = !intParam->hexMode;

		slider->valueFromTextFunction = intParam->hexMode ? &IntStepperUI::textToHexValue : nullptr;
		slider->textFromValueFunction = intParam->hexMode ? &IntStepperUI::hexValueToText : nullptr;
		slider->setNumDecimalPlacesToDisplay(0);
		slider->updateText();
	}
}

void IntStepperUI::updateUIParamsInternal()
{
	FloatStepperUI::updateUIParamsInternal();
	slider->setNumDecimalPlacesToDisplay(0);
}
