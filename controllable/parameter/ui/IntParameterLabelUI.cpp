/*
  ==============================================================================

    IntParameterLabelUI.cpp
    Created: 10 Dec 2016 10:51:19am
    Author:  Ben

  ==============================================================================
*/

IntParameterLabelUI::IntParameterLabelUI(Parameter * p) :
	FloatParameterLabelUI(p)
{

}
void IntParameterLabelUI::labelTextChanged(Label *)
{
	//String  originalString = valueLabel.getText().substring(prefix.length(), valueLabel.getText().length() - suffix.length());
	parameter->setValue((int)(valueLabel.getText().getFloatValue()));
}