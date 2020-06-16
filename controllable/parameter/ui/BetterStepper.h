/*
  ==============================================================================

    BetterIntStepper.h
    Created: 28 Sep 2016 3:14:10pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BetterStepper : public Slider
{
public:
	BetterStepper(const String &tooltip);
	virtual ~BetterStepper();

	virtual void setTooltip(const String &tooltip) override;
};