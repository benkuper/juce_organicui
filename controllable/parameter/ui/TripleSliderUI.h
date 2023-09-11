/*
  ==============================================================================

	TripleSliderUI.h
	Created: 25 Oct 2016 11:46:46am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class TripleSliderUI : public ParameterUI
{

public:
	TripleSliderUI(juce::Array<Point3DParameter*>parameters);
	virtual ~TripleSliderUI();

	enum Direction { HORIZONTAL, VERTICAL };

	juce::Array<Point3DParameter*> p3ds;
	Point3DParameter* p3d;

	FloatParameter xParam;
	FloatParameter yParam;
	FloatParameter zParam;

	std::unique_ptr<ParameterUI> xSlider;
	std::unique_ptr<ParameterUI> ySlider;
	std::unique_ptr<ParameterUI> zSlider;

	juce::var mouseDownValue;

	bool isUpdatingFromParam;

	void mouseDownInternal(const juce::MouseEvent&) override;
	void mouseUpInternal(const juce::MouseEvent&) override;

	void paint(juce::Graphics& g) override;
	void resized() override;
	void showEditWindowInternal() override;
	void showEditRangeWindowInternal() override;

	virtual void rangeChanged(Parameter* p) override;

protected:
	void updateUIParamsInternal() override;
	virtual void newMessage(const Parameter::ParameterEvent& e) override;

	//void valueChanged(const var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TripleSliderUI)
};