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
	GradientColor(float time = 0, const juce::Colour & color = juce::Colours::black, const juce::String &name = "Color");
	virtual ~GradientColor();

	enum Interpolation { LINEAR, NONE };
	FloatParameter * position;
	EnumParameter * interpolation;

	void setMovePositionReferenceInternal() override;
	void setPosition(juce::Point<float> targetTime) override;
	juce::Point<float> getPosition() override;
	void addUndoableMoveAction(juce::Array<juce::UndoableAction*>& actions) override;

	int gradientIndex;

	DECLARE_TYPE("GradientColor");
	DECLARE_UI_FUNC;

};