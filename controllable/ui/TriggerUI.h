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
    TriggerUI(Trigger * trigger);
    virtual ~TriggerUI();

    WeakReference<Trigger>  trigger;

	bool useCustomBGColor;
	Colour customBGColor;

    // Inherited via AsyncListener
	void newMessage(const WeakReference<Trigger> &) override;
	
	// should be inherited
    virtual void triggerTriggered(const Trigger * p) =0;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerUI)


};