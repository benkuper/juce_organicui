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
	TripleSliderUI(Point3DParameter* parameter = nullptr);
	virtual ~TripleSliderUI();

	enum Direction { HORIZONTAL, VERTICAL };

	Point3DParameter* p3d;

	FloatParameter xParam;
	FloatParameter yParam;
	FloatParameter zParam;

	std::unique_ptr<ParameterUI> xSlider;
	std::unique_ptr<ParameterUI> ySlider;
	std::unique_ptr<ParameterUI> zSlider;

	var mouseDownValue;

	bool isUpdatingFromParam;

	void mouseDownInternal(const MouseEvent&) override;
	void mouseUpInternal(const MouseEvent&) override;

	void paint(Graphics& g) override;
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