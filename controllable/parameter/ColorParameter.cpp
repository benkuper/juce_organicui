#include "ColorParameter.h"
/*
  ==============================================================================

	ColorParameter.cpp
	Created: 11 Apr 2017 9:33:55am
	Author:  Ben

  ==============================================================================
*/


ColorParameter::ColorParameter(const String & niceName, const String & description, const Colour & initialColor, bool enabled) :
	Parameter(COLOR,niceName,description,var(),var(),var(),enabled),
	mode(FLOAT)
{
	//lockManualControlMode = true;
	canBeAutomated = true;

	setColor(initialColor);
	defaultValue = value;
	resetValue();
}

ColorParameter::~ColorParameter() {}

const Colour ColorParameter::getColor()
{
	if (value.size() < 4)
	{
		DBG("Error, color value size is " << value.size());
		return Colours::black;
	}

	if (mode == FLOAT) return Colour((uint8)((float)value[0]*255), (uint8)((float)value[1]*255), (uint8)((float)value[2]*255), (uint8)((float)value[3]*255));
	else return Colour((uint8)(int)value[0], (uint8)(int)value[1], (uint8)(int)value[2], (uint8)(int)value[3]);
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
	if (mode == FLOAT)
	{
		colorVar.append(_color.getFloatRed());
		colorVar.append(_color.getFloatGreen());
		colorVar.append(_color.getFloatBlue());
		colorVar.append(_color.getFloatAlpha());
	}
	else
	{
		colorVar.append(_color.getRed());
		colorVar.append(_color.getGreen());
		colorVar.append(_color.getBlue());
		colorVar.append(_color.getAlpha());
	}
	
	setValue(colorVar, silentSet, force);
}

StringArray ColorParameter::getValuesNames()
{
	return StringArray("Red", "Green", "Blue", "Alpha");
}

bool ColorParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	if (!(newValue.isArray() && oldValue.isArray())) return false;

	valueSetLock.enter();
	bool result = newValue[0] == oldValue[0] && newValue[1] == oldValue[1] && newValue[2] == oldValue[2] && oldValue[3] == newValue[3];
	valueSetLock.exit();

	return result;
}

var ColorParameter::getLerpValueTo(var targetValue, float weight)
{
	if (!targetValue.isArray()) return value;
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

	for (int i = 0; i < values.size(); i++)
	{
		jassert(values[i].size() == 4);
		for (int j = 0; j < 4; j++)
		{
			tValues[j] += (float)values[i][j] * weights[i];
		}
	}

	setFloatRGBA(tValues[0], tValues[1], tValues[2], tValues[3]);
}

void ColorParameter::setControlAutomation()
{
	automation.reset(new ParameterColorAutomation(this, !isLoadingData));
}

ColorParameterUI * ColorParameter::createColorParamUI()
{
	return new ColorParameterUI(this);
}

ControllableUI * ColorParameter::createDefaultUI(Controllable *)
{
	return createColorParamUI();
}
