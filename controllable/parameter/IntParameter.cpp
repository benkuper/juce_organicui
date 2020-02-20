/*
  ==============================================================================

	IntParameter.cpp
	Created: 8 Mar 2016 1:22:23pm
	Author:  bkupe

  ==============================================================================
*/



IntParameter::IntParameter(const String& niceName, const String& description, const int& initialValue, const int& minValue, const int& maxValue, bool enabled) :
	Parameter(Type::INT, niceName, description, initialValue, minValue, maxValue, enabled),
	hexMode(false)
{
	canHaveRange = true;
	canBeAutomated = true;
	argumentsDescription = "int";
}

void IntParameter::setValueInternal(var & _value)
{
	if ((int)_value < (int)minimumValue && autoAdaptRange) setRange(_value, maximumValue, false);
	else if ((int)_value > (int)maximumValue && autoAdaptRange) setRange(minimumValue, _value, false);

	if ((int)minimumValue > (int)maximumValue) return;
	this->value = jlimit<int>(minimumValue, maximumValue, _value);
}

var IntParameter::getLerpValueTo(var targetValue, float weight)
{
	return (int)jmap(weight, floatValue(), (float)targetValue);
}

void IntParameter::setWeightedValue(Array<var> values, Array<float> weights)
{
	jassert(values.size() == weights.size());

	float tValue = 0;

	for (int i = 0; i < values.size(); i++)
	{
		tValue += (float)values[i] * weights[i];
	}

	setValue((int)tValue);
} 


bool IntParameter::hasRange()
{
	return (float)minimumValue != INT32_MIN && (float)maximumValue != INT32_MAX;

}

void IntParameter::setControlAutomation()
{
	automation.reset(new ParameterNumberAutomation(this, !isLoadingData));
}

var IntParameter::getJSONDataInternal()
{
	var data = Parameter::getJSONDataInternal();
	data.getDynamicObject()->setProperty("hexMode", hexMode);
	return data;
}

void IntParameter::loadJSONDataInternal(var data)
{
	Parameter::loadJSONDataInternal(data);
	hexMode = data.getProperty("hexMode", false);
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

ControllableUI * IntParameter::createDefaultUI(Controllable * targetControllable) {

	IntParameter * p = dynamic_cast<IntParameter *>(targetControllable);
	ParameterUI * pui = createStepper(p);
	//if (/*isControllableFeedbackOnly || !enabled || */ !hasRange()) pui = createLabelUI(p);
	//else pui = createStepper(p);

	return pui;
};
