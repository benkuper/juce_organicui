/*
  ==============================================================================

    IntSliderUI.cpp
    Created: 8 Mar 2016 3:46:34pm
    Author:  bkupe

  ==============================================================================
*/

IntSliderUI::IntSliderUI(Array<Parameter*> parameters) :
    FloatSliderUI(parameters)
{
	fixedDecimals = 0;
}

IntSliderUI::~IntSliderUI()
{
}

void IntSliderUI::setParamNormalizedValue(float value)
{
	FloatSliderUI::setParamNormalizedValue(value);
}

float IntSliderUI::getParamNormalizedValue()
{
    return FloatSliderUI::getParamNormalizedValue();
}
