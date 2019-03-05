/*
  ==============================================================================

    IntSliderUI.cpp
    Created: 8 Mar 2016 3:46:34pm
    Author:  bkupe

  ==============================================================================
*/

IntSliderUI::IntSliderUI(Parameter * parameter) :
    FloatSliderUI(parameter)
{
	fixedDecimals = 0;
}

IntSliderUI::~IntSliderUI()
{
}

void IntSliderUI::setParamNormalizedValue(float value)
{
    parameter->setNormalizedValue(value);
}

float IntSliderUI::getParamNormalizedValue()
{
    return parameter->getNormalizedValue();
}
