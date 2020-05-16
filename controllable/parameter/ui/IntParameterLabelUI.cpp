#include "IntParameterLabelUI.h"
/*
  ==============================================================================

    IntParameterLabelUI.cpp
    Created: 10 Dec 2016 10:51:19am
    Author:  Ben

  ==============================================================================
*/

IntParameterLabelUI::IntParameterLabelUI(Parameter * p) :
	FloatParameterLabelUI(p),
	intParam(dynamic_cast<IntParameter *>(p))
{
	valueChanged(parameter->getValue());
}
void IntParameterLabelUI::labelTextChanged(Label *)
{
	//String  originalString = valueLabel.getText().substring(prefix.length(), valueLabel.getText().length() - suffix.length());
	parameter->setValue(intParam->hexMode? valueLabel.getText().getHexValue32():(int)(valueLabel.getText().getFloatValue()));
}

void IntParameterLabelUI::valueChanged(const var& v)
{
	if (intParam->hexMode)
	{
		shouldUpdateLabel = true;
	}
	else
	{
		FloatParameterLabelUI::valueChanged(v);
	}
}

String IntParameterLabelUI::getValueString(const var &val) const
{
	return "0x" + String::toHexString(intParam->intValue()).toUpperCase();
}
