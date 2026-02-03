/*
  ==============================================================================

	IntParameter.cpp
	Created: 8 Mar 2016 1:22:23pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"


IntParameter::IntParameter(const String& niceName, const String& description, const int& initialValue, const int& minValue, const int& maxValue, bool enabled) :
	Parameter(Type::INT, niceName, description, initialValue, minValue, maxValue, enabled),
	hexMode(false)
{
	canHaveRange = true;
	rebuildUIOnRangeChange = false;
	canBeAutomated = true;
	argumentsDescription = "int";
}

void IntParameter::setValueInternal(var& _value)
{
	if (_value.isObject() || _value.isArray()) return;
	value = _value.isString() ? _value.toString().getIntValue() : (int)_value;
}

var IntParameter::getLerpValueTo(var targetValue, float weight)
{
	return (int)jmap(weight, floatValue(), (float)targetValue);
}

void IntParameter::setWeightedValue(Array<var> values, Array<float> weights)
{
	jassert(values.size() == weights.size());

	float tValue = 0;

	for (int i = 0; i < values.size(); ++i)
	{
		tValue += (float)values[i] * weights[i];
	}

	setValue((int)tValue);
}


bool IntParameter::hasRange() const
{
	return (float)minimumValue != INT32_MIN || (float)maximumValue != INT32_MAX;

}

void IntParameter::setControlAutomation()
{
	automation.reset(new ParameterNumberAutomation(this, !isLoadingData));
}

bool IntParameter::setAttributeInternal(String attribute, var val)
{
	if (attribute == "hexMode") hexMode = val;
	else
	{
		return Parameter::setAttributeInternal(attribute, val);
	}

	return true;
}

juce::var IntParameter::getAttributeInternal(juce::String name) const
{
	if (name == "hexMode")
	{
		return hexMode;
	}
	return juce::var();
}


StringArray IntParameter::getValidAttributes() const
{
	StringArray att = Parameter::getValidAttributes();
	att.addArray({ "hexMode" });
	return att;
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

IntSliderUI* IntParameter::createSlider(Array<Parameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new IntSliderUI(parameters);
}

IntStepperUI* IntParameter::createStepper(Array<Parameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new IntStepperUI(parameters);
}

IntParameterLabelUI* IntParameter::createLabelUI(Array<Parameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new IntParameterLabelUI(parameters);
}

ControllableUI* IntParameter::createDefaultUI(Array<Controllable*> controllables) {

	ParameterUI* pui = createStepper(getArrayAs<Controllable, Parameter>(controllables));
	//if (/*isControllableFeedbackOnly || !enabled || */ !hasRange()) pui = createLabelUI(p);
	//else pui = createStepper(p);

	return pui;
}

var IntParameter::getCroppedValue(var originalValue)
{
	return (int)jlimit<float>(minimumValue, maximumValue, originalValue);
}
