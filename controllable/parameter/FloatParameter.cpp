/*
  ==============================================================================

	FloatParameter.cpp
	Created: 8 Mar 2016 1:22:10pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

FloatParameter::FloatParameter(const String& niceName, const String& description, const double& initialValue, const double& minValue, const double& maxValue, bool enabled) :
	Parameter(Type::FLOAT, niceName, description, initialValue, minValue, maxValue, enabled),
	unitSteps(0),
	stringDecimals(DEFAULT_STRING_DECIMALS),
	defaultUI(NONE),
	customUI(NONE)
{
	canHaveRange = true;
	canBeAutomated = true;
	argumentsDescription = "float";
}


FloatSliderUI* FloatParameter::createSlider(Array<Parameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new FloatSliderUI(parameters);
}

FloatStepperUI* FloatParameter::createStepper(Array<Parameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new FloatStepperUI(parameters);
}

FloatParameterLabelUI* FloatParameter::createLabelParameter(Array<Parameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new FloatParameterLabelUI(parameters);
}

TimeLabel* FloatParameter::createTimeLabelParameter(Array<Parameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new TimeLabel(parameters);
}

ControllableUI* FloatParameter::createDefaultUI(Array<Controllable*> controllables) {
	UIType t = customUI != NONE ? customUI : defaultUI;

	if (t == NONE) t = hasRange() ? SLIDER : LABEL;

	Array<Parameter*> parameters = getArrayAs<Controllable, Parameter>(controllables);

	switch (t)
	{
	case NONE:
		break;

	case SLIDER:
		return createSlider(parameters);
		break;
	case STEPPER:
		return createStepper(parameters);
		break;
	case LABEL:
		return createLabelParameter(parameters);
		break;
	case TIME:
		return createTimeLabelParameter(parameters);
		break;
	}

	jassert(false);
	return nullptr;
}

bool FloatParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	return jlimit<double>(minimumValue, maximumValue, newValue) == (double)oldValue;
}


void FloatParameter::setValueInternal(var& val)
{
	if (unitSteps > 0) value = getStepSnappedValueFor(val);
	else Parameter::setValueInternal(val);
}

bool FloatParameter::hasRange()
{
	return (double)minimumValue != (double)INT32_MIN && (double)maximumValue != (double)INT32_MAX;
}


var FloatParameter::getLerpValueTo(var targetValue, float weight)
{
	return jmap<double>(weight, doubleValue(), (double)targetValue);
}

void FloatParameter::setWeightedValue(Array<var> values, Array<float> weights)
{
	jassert(values.size() == weights.size());

	double tValue = 0;

	for (int i = 0; i < values.size(); ++i)
	{
		tValue += (double)values[i] * weights[i];
	}

	setValue(tValue);
}

double FloatParameter::getStepSnappedValueFor(double originalValue)
{
	return unitSteps == 0 ? originalValue : round(originalValue * unitSteps) / unitSteps;
}

void FloatParameter::setControlAutomation()
{
	automation.reset(new ParameterNumberAutomation(this, !isLoadingData));
}

String FloatParameter::stringValue()
{
	return String(doubleValue(), stringDecimals);
}

bool FloatParameter::setAttributeInternal(String attribute, var val)
{
	if (attribute == "ui")
	{
		if (val == "time") defaultUI = TIME;
		else if (val == "slider") defaultUI = SLIDER;
		else if (val == "stepper") defaultUI = STEPPER;
		else if (val == "label") defaultUI = LABEL;
	}
	else if (attribute == "unitSteps")
	{
		unitSteps = (double)val;
	}
	else if (attribute == "stringDecimals")
	{
		stringDecimals = (int)val;
	}
	else
	{
		return Parameter::setAttributeInternal(attribute, val);
	}

	return true;
}

StringArray FloatParameter::getValidAttributes() const
{
	StringArray att = Parameter::getValidAttributes();
	att.addArray({ "ui", "unitSteps", "stringDecimals" });
	return att;
}

var FloatParameter::getJSONDataInternal()
{
	var data = Parameter::getJSONDataInternal();
	if (customUI != NONE) data.getDynamicObject()->setProperty("customUI", customUI);
 	if(stringDecimals != DEFAULT_STRING_DECIMALS) data.getDynamicObject()->setProperty("stringDecimals", stringDecimals);
	return data;
}

void FloatParameter::loadJSONDataInternal(var data)
{
	Parameter::loadJSONDataInternal(data);
	if (data.getDynamicObject()->hasProperty("defaultUI")) defaultUI = (UIType)(int)data.getProperty("defaultUI", SLIDER);
	customUI = (UIType)(int)data.getProperty("customUI", NONE);
	stringDecimals = (int)data.getProperty("stringDecimals", DEFAULT_STRING_DECIMALS);
}

var FloatParameter::getCroppedValue(var originalValue)
{
	double v = isnan((double)originalValue) ? 0.0f : (double)originalValue;
	return jlimit<double>(minimumValue, maximumValue, v);
}

