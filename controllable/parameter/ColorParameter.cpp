/*
  ==============================================================================

	ColorParameter.cpp
	Created: 11 Apr 2017 9:33:55am
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"


ColorParameter::ColorParameter(const String & niceName, const String & description, const Colour & initialColor, bool enabled) :
	Parameter(COLOR,niceName,description,var(),var(),var(),enabled)
{
	//lockManualControlMode = true;

	canBeAutomated = true;

	var minVal;
	var maxVal;
	for (int i = 0; i < 4; ++i)
	{
		minVal.append(0);
		maxVal.append(1.0f);
	}

	minimumValue = minVal;
	maximumValue = maxVal;

	defaultValue = var();
	defaultValue.append(initialColor.getFloatRed());
	defaultValue.append(initialColor.getFloatGreen());
	defaultValue.append(initialColor.getFloatBlue());
	defaultValue.append(initialColor.getFloatAlpha());

	value = defaultValue.clone();

}

ColorParameter::~ColorParameter() {}

const Colour ColorParameter::getColor()
{
	GenericScopedLock<SpinLock> lock(valueSetLock);

	if (!value.isArray()) return Colours::black;
	while (value.size() < 4) value.append(0);

	return Colour::fromFloatRGBA((float)value[0], (float)value[1], (float)value[2], (float)value[3]);
}

void ColorParameter::setFloatRGBA(const float & r, const float & g, const float & b, const float & a)
{
	setColor(Colour::fromFloatRGBA(r, g, b, a));
}

void ColorParameter::setColor(const uint32 & _color, bool silentSet, bool force)
{
	setColor(Colour(_color),silentSet,force);
}

void ColorParameter::setColor(const Colour &_color, bool silentSet, bool force)
{
	var colorVar;
	colorVar.append(_color.getFloatRed());
	colorVar.append(_color.getFloatGreen());
	colorVar.append(_color.getFloatBlue());
	colorVar.append(_color.getFloatAlpha());
	
	setValue(colorVar, silentSet, force);
}

void ColorParameter::setDefaultValue(const Colour& _color, bool doResetValue)
{
	var cVal = var();
	cVal.append(_color.getFloatRed());
	cVal.append(_color.getFloatGreen());
	cVal.append(_color.getFloatBlue());
	cVal.append(_color.getFloatAlpha());

	Parameter::setDefaultValue(cVal, doResetValue);
}

StringArray ColorParameter::getValuesNames()
{
	return StringArray("Red", "Green", "Blue", "Alpha");
}

bool ColorParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	if (!(newValue.isArray() && oldValue.isArray())) return false;
	if (newValue.size() != 4 || oldValue.size() != 4) return false;

	bool result = newValue[0] == oldValue[0] && newValue[1] == oldValue[1] && newValue[2] == oldValue[2] && oldValue[3] == newValue[3];

	return result;
}

var ColorParameter::getLerpValueTo(var targetValue, float weight)
{
	if (!targetValue.isArray()) return value;
	GenericScopedLock<SpinLock> lock(valueSetLock);
	var result;
	result.append(jmap(weight, (float)value[0], (float)targetValue[0]));
	result.append(jmap(weight, (float)value[1], (float)targetValue[1]));
	result.append(jmap(weight, (float)value[2], (float)targetValue[2]));
	result.append(jmap(weight, (float)value[3], (float)targetValue[3]));
	return result;
}

void ColorParameter::setWeightedValue(Array<var> values, Array<float> weights)
{
	jassert(values.size() == weights.size());

	float tValues[4];
	tValues[0] = tValues[1] = tValues[2] = tValues[3] = 0;

	{
		GenericScopedLock<SpinLock> lock(valueSetLock);
		for (int i = 0; i < values.size(); ++i)
		{
			jassert(values[i].size() == 4);
			for (int j = 0; j < 4; j++)
			{
				tValues[j] += (float)values[i][j] * weights[i];
			}
		}
	}

	setFloatRGBA(tValues[0], tValues[1], tValues[2], tValues[3]);
}

void ColorParameter::setControlAutomation()
{
	automation.reset(new ParameterColorAutomation(this, !isLoadingData));
}

var ColorParameter::getRemoteControlValue()
{
	return getColor().toString();
}

ColorParameterUI * ColorParameter::createColorParamUI(Array<ColorParameter *> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new ColorParameterUI(parameters);
}

ControllableUI * ColorParameter::createDefaultUI(Array<Controllable*> controllables)
{
	return createColorParamUI(getArrayAs<Controllable, ColorParameter>(controllables));
}
