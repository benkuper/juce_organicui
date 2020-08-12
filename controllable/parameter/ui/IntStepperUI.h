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
    IntStepperUI(Parameter * parameter);
    virtual ~IntStepperUI();

	IntParameter* intParam;

	static bool showHexModeOption;

	static String hexValueToText(double v);
	static double textToHexValue(String t);

	void addPopupMenuItemsInternal(PopupMenu* p) override;
	void handleMenuSelectedID(int result) override;

protected:
	void updateUIParamsInternal() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntStepperUI)
};