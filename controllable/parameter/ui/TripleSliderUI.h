/*
  ==============================================================================

    TripleSliderUI.h
    Created: 2 Nov 2016 4:17:34pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class TripleSliderUI : 
	public ParameterUI
{

public:
	TripleSliderUI(Point3DParameter * parameter = nullptr);
	virtual ~TripleSliderUI();

	enum Direction { HORIZONTAL, VERTICAL };

	Point3DParameter * p3d;

	FloatParameter xParam;
	FloatParameter yParam;
	FloatParameter zParam;

	std::unique_ptr<ControllableUI> xSlider;
	std::unique_ptr<ControllableUI> ySlider;
	std::unique_ptr<ControllableUI> zSlider;

	var mouseDownValue;

	void updateUIParamsInternal() override;

	void paint(Graphics& g) override;
	void resized() override;

	void mouseDownInternal(const MouseEvent &) override;
	void mouseUpInternal(const MouseEvent &) override;

	void showEditWindowInternal() override;
	void showEditRangeWindowInternal() override;

	virtual void rangeChanged(Parameter *) override;
protected:
	virtual void newMessage(const Parameter::ParameterEvent &e) override;

	//void valueChanged(const var &) override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TripleSliderUI)
};