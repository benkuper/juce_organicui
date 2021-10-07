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

	std::unique_ptr<ParameterUI> xSlider;
	std::unique_ptr<ParameterUI> ySlider;

	std::unique_ptr<P2DUI> canvasUI;
	juce::Rectangle<float> canvasSwitchRect;

	var mouseDownValue;

	bool isUpdatingFromParam;


	void mouseDownInternal(const MouseEvent &) override;
	void mouseUpInternal(const MouseEvent &) override;

	void paint(Graphics& g) override;
    void resized() override;
	void showEditWindowInternal() override;
	void showEditRangeWindowInternal() override;

	void updateUseExtendedEditor();

	virtual void rangeChanged(Parameter * p) override;

protected:
	void updateUIParamsInternal() override;
	virtual void newMessage(const Parameter::ParameterEvent & e) override;

	//void valueChanged(const var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoubleSliderUI)
};