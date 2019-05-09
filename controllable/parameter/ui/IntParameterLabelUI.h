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
	IntParameterLabelUI(Parameter * p);
	virtual ~IntParameterLabelUI() {};

protected:
	virtual void labelTextChanged(Label * labelThatHasChanged) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntParameterLabelUI)

};