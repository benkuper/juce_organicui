/*
  ==============================================================================

    FloatStepperUI.h
    Created: 28 Apr 2016 6:00:28pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class FloatStepperUI : 
	public ParameterUI, 
	public Slider::Listener,
    public Timer
{

public:
    FloatStepperUI(Parameter * _parameter);
    virtual ~FloatStepperUI();

    std::unique_ptr<BetterStepper> slider;

    bool shouldUpdateStepper;
    
    void resized() override;
    
    void timerCallback() override;
    
protected:
    void valueChanged(const var &) override;
    void rangeChanged(Parameter * p) override;

	virtual void feedbackStateChanged() override;

    // Inherited via Listener
    virtual void sliderValueChanged(Slider * slider) override;

	virtual void controlModeChanged(Parameter *) override;

	virtual void controllableStateChanged() override;
};