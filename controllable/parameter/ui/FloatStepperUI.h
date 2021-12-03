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
    FloatStepperUI(Array<Parameter *> parameters);
    virtual ~FloatStepperUI();

    std::unique_ptr<BetterStepper> slider;

    bool shouldUpdateStepper;
    float valueAtDragStart;
    
	void paint(Graphics& g) override;
    void resized() override;

    void updateUIParamsInternal() override;
    void timerCallback() override;
    
protected:
    void valueChanged(const var &) override;
    void rangeChanged(Parameter * p) override;

    // Inherited via Listener
    virtual void sliderValueChanged(Slider * slider) override;
    virtual void sliderDragStarted(Slider* slider) override;
    virtual void sliderDragEnded(Slider* slider) override;
};