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
	public Timer
{
public:
    TriggerButtonUI(Trigger * t);
    ~TriggerButtonUI();

	bool drawTriggering;

    void paint (Graphics&) override;
    void triggerTriggered(const Trigger * p) override ;

	void mouseDownInternal(const MouseEvent &e) override;

	// Inherited via Timer
	virtual void timerCallback() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriggerButtonUI)
};

