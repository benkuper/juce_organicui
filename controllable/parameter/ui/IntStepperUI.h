/*
  ==============================================================================

    IntStepperUI.h
    Created: 8 Mar 2016 3:46:43pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class IntStepperUI : public FloatStepperUI 
{
public:
    IntStepperUI(juce::Array<Parameter*> parameters);
    virtual ~IntStepperUI();

	juce::Array<IntParameter*> intParams;
	IntParameter* intParam;

	static bool showHexModeOption;

	static juce::String hexValueToText(double v);
	static double textToHexValue(juce::String t);

	void addPopupMenuItemsInternal(juce::PopupMenu* p) override;
	void handleMenuSelectedID(int result) override;

protected:
	void updateUIParamsInternal() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntStepperUI)
};