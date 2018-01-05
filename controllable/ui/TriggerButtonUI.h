/*
  ==============================================================================

    TriggerButtonUI.h
    Created: 8 Mar 2016 3:45:53pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef TRIGGERBUTTONUI_H_INCLUDED
#define TRIGGERBUTTONUI_H_INCLUDED



//==============================================================================
/*
*/


class TriggerButtonUI : 
	public TriggerUI
{
public:
    TriggerButtonUI(Trigger * t);
    ~TriggerButtonUI();

	Rectangle<int> drawRect;

    void paint (Graphics&) override;
    void triggerTriggered(const Trigger * p) override ;

	void mouseDownInternal(const MouseEvent &e) override;

	bool hitTest(int x, int y) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriggerButtonUI)
};


#endif  // TRIGGERBUTTONUI_H_INCLUDED
