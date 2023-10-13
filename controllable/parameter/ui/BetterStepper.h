/*
  ==============================================================================

    BetterIntStepper.h
    Created: 28 Sep 2016 3:14:10pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BetterStepper : public juce::Slider
{
public:
	BetterStepper(const juce::String &tooltip);
	virtual ~BetterStepper();

	virtual void setTooltip(const juce::String &tooltip) override;
};