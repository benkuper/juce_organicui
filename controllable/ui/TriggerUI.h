/*
  ==============================================================================

    TriggerUI.h
    Created: 8 Mar 2016 3:48:52pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class TriggerUI : 
	public ControllableUI, 
	public Trigger::AsyncListener
{
public :
    TriggerUI(juce::Array<Trigger *> triggers);
    virtual ~TriggerUI();

	juce::Array<juce::WeakReference<Trigger>> triggers;
	juce::WeakReference<Trigger> trigger;

	bool useCustomBGColor;
	juce::Colour customBGColor;

    // Inherited via AsyncListener
	void newMessage(const juce::WeakReference<Trigger> &) override;
	
	// should be inherited
    virtual void triggerTriggered(const Trigger * p) =0;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerUI)


};