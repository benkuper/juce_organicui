/*
  ==============================================================================

    IntParameter.cpp
    Created: 8 Mar 2016 1:22:23pm
    Author:  bkupe

  ==============================================================================
*/



IntParameter::IntParameter(const String & niceName, const String &description, const int & initialValue, const int & minValue, const int & maxValue, bool enabled) :
    Parameter(Type::INT, niceName, description, initialValue, minValue, maxValue, enabled)
{
	argumentsDescription = "int";
}

void IntParameter::setValueInternal(var & _value)
{
	if ((int)_value < (int)minimumValue && autoAdaptRange) setRange(_value, maximumValue,false);
	else if ((int)_value >(int)maximumValue && autoAdaptRange) setRange(minimumValue, _value, false);

	if ((int)minimumValue > (int)maximumValue) return;
	this->value = jlimit<int>(minimumValue, maximumValue, _value);
}

IntSliderUI * IntParameter::createSlider(IntParameter * target)
{
	if (target == nullptr) target = this;
	return new IntSliderUI(target);
}

IntStepperUI * IntParameter::createStepper(IntParameter * target)
{
	if (target == nullptr) target = this;
	return new IntStepperUI(target);
}

IntParameterLabelUI * IntParameter::createLabelUI(IntParameter * target)
{
	if (target == nullptr) target = this;
	return new IntParameterLabelUI(target);
}

ControllableUI * IntParameter::createDefaultUI(Controllable * targetControllable){

	return createStepper(dynamic_cast<IntParameter *>(targetControllable));
};
