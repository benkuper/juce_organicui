/*
  ==============================================================================

    IntParameterLabelUI.h
    Created: 10 Dec 2016 10:51:19am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class IntParameterLabelUI :
	public FloatParameterLabelUI 
{
public:
	IntParameterLabelUI(juce::Array<Parameter *> parameters);
	virtual ~IntParameterLabelUI() {};

	IntParameter* intParam;

protected:
	virtual void labelTextChanged(juce::Label * labelThatHasChanged) override;
	virtual void valueChanged(const juce::var& v) override;
	juce::String getValueString(const juce::var &val) const override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntParameterLabelUI)

};