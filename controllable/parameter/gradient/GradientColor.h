/*
  ==============================================================================

    TimeColor.h
    Created: 11 Apr 2017 11:40:21am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class GradientColor :
	public BaseItem
{
public:
	GradientColor(float time = 0, const Colour & color = Colours::black, const String &name = "Color");
	virtual ~GradientColor();

	enum Interpolation { LINEAR, NONE };
	ColorParameter * color;
	FloatParameter * position;
	EnumParameter * interpolation;

	int gradientIndex;
};