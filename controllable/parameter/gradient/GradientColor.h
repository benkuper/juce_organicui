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

	void setMovePositionReferenceInternal() override;
	void setPosition(Point<float> targetTime) override;
	Point<float> getPosition() override;
	void addUndoableMoveAction(Array<UndoableAction*>& actions) override;

	int gradientIndex;
};