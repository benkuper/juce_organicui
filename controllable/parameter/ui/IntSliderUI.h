/*
  ==============================================================================

    IntSliderUI.h
    Created: 8 Mar 2016 3:46:34pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class IntSliderUI : public FloatSliderUI
{
public:
    IntSliderUI(juce::Array<Parameter*> parameters);
    virtual ~IntSliderUI();

    void setParamNormalizedValue(float value) override;
    float getParamNormalizedValue() override;

protected:
    void valueChanged(const juce::var&) override { repaint(); };

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntSliderUI)
};