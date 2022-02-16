/*
  ==============================================================================

	IntParameterLabelUI.cpp
	Created: 10 Dec 2016 10:51:19am
	Author:  Ben

  ==============================================================================
*/

IntParameterLabelUI::IntParameterLabelUI(Array<Parameter*> parameters) :
	FloatParameterLabelUI(parameters),
	intParam(dynamic_cast<IntParameter*>(parameters[0]))
{
	valueChanged(parameter->getValue());
}
void IntParameterLabelUI::labelTextChanged(Label*)
{
	//String  originalString = valueLabel.getText().substring(prefix.length(), valueLabel.getText().length() - suffix.length());
	parameter->setValue(intParam->hexMode ? valueLabel.getText().getHexValue32() : (int)(valueLabel.getText().getFloatValue()));
}

void IntParameterLabelUI::valueChanged(const var& v)
{
	if (intParam->hexMode)
	{
		shouldRepaint = true;
	}
	else
	{
		FloatParameterLabelUI::valueChanged(v);
	}
}

String IntParameterLabelUI::getValueString(const var& val) const
{
	return intParam->hexMode ? "0x" + String::toHexString(intParam->intValue()).toUpperCase() : String(intParam->intValue());
}
