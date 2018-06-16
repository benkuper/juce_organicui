/*
  ==============================================================================

    FloatStepperUI.h
    Created: 28 Apr 2016 6:00:28pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef FLOATSTEPPERUI_H_INCLUDED
#define FLOATSTEPPERUI_H_INCLUDED

class FloatStepperUI : 
	public ParameterUI, 
	public Slider::Listener
{

public:
    FloatStepperUI(Parameter * _parameter);
    virtual ~FloatStepperUI();

    ScopedPointer<BetterStepper> slider;

    void resized() override;

protected:
    void valueChanged(const var &) override;
    void rangeChanged(Parameter * p) override;

	void feedbackStateChanged() override;

    // Inherited via Listener
    virtual void sliderValueChanged(Slider * slider) override;

	virtual void controllableStateChanged() override;
};


#endif  // FLOATSTEPPERUI_H_INCLUDED
