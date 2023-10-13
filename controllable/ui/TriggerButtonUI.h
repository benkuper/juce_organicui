/*
  ==============================================================================

    TriggerButtonUI.h
    Created: 8 Mar 2016 3:45:53pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class TriggerButtonUI : 
	public TriggerUI,
	public juce::Timer
{
public:
    TriggerButtonUI(juce::Array<Trigger *> t);
    ~TriggerButtonUI();

	bool drawTriggering;
    juce::Rectangle<float> hitRect;

    void paint (juce::Graphics&) override;
    void triggerTriggered(const Trigger * p) override ;

	void mouseDownInternal(const juce::MouseEvent &e) override;

    bool hitTest(int x, int y) override;

	// Inherited via Timer
	virtual void timerCallback() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriggerButtonUI)
};

