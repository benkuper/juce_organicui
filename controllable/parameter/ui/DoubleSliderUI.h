/*
  ==============================================================================

    DoubleSliderUI.h
    Created: 25 Oct 2016 11:46:46am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class DoubleSliderUI : public ParameterUI
{

public:
	DoubleSliderUI(Point2DParameter * parameter = nullptr);
	virtual ~DoubleSliderUI();

	enum Direction { HORIZONTAL, VERTICAL };
	
	Point2DParameter * p2d;

	FloatParameter xParam;
	FloatParameter yParam;

	ScopedPointer<ParameterUI> xSlider;
	ScopedPointer<ParameterUI> ySlider;

	var mouseDownValue;

	void mouseDownInternal(const MouseEvent &) override;
	void mouseUpInternal(const MouseEvent &) override;

    void resized() override;
	void showEditWindow() override;
	void showEditRangeWindow() override;

	virtual void rangeChanged(Parameter * p) override;

protected:
	void feedbackStateChanged() override;
	virtual void newMessage(const Parameter::ParameterEvent & e) override;

	//void valueChanged(const var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoubleSliderUI)
};