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
	public juce::Slider::Listener
{

public:
    FloatStepperUI(juce::Array<Parameter *> parameters);
    virtual ~FloatStepperUI();

    std::unique_ptr<BetterStepper> slider;

    float valueAtDragStart;
    
	void paint(juce::Graphics& g) override;
    void resized() override;

    void updateUIParamsInternal() override;
    void handlePaintTimerInternal() override;
    
protected:
    void valueChanged(const juce::var &) override;
    void rangeChanged(Parameter * p) override;

    // Inherited via Listener
    virtual void sliderValueChanged(juce::Slider * slider) override;
    virtual void sliderDragStarted(juce::Slider* slider) override;
    virtual void sliderDragEnded(juce::Slider* slider) override;
};