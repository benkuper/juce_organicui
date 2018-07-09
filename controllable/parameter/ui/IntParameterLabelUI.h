/*
  ==============================================================================

    IntParameterLabelUI.h
    Created: 10 Dec 2016 10:51:19am
    Author:  Ben

  ==============================================================================
*/

#ifndef INTPARAMETERLABELUI_H_INCLUDED
#define INTPARAMETERLABELUI_H_INCLUDED

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

#endif  // INTPARAMETERLABELUI_H_INCLUDED
