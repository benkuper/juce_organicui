/*
  ==============================================================================

	P2DUI.h
	Created: 25 Oct 2016 11:46:46am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class P2DUI :
	public ParameterUI
{

public:
	P2DUI(juce::Array<Point2DParameter*> parameters);
	virtual ~P2DUI();

	juce::Array<Point2DParameter*> p2ds;
	Point2DParameter* p2d;

	juce::var mouseDownNormalizedValue;
	juce::var mouseDownValue;

	juce::Rectangle<float> canvasRect;

	void mouseDownInternal(const juce::MouseEvent&) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUpInternal(const juce::MouseEvent&) override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	virtual void valueChanged(const juce::var& v) override;
	virtual void rangeChanged(Parameter* p) override;

	void addPopupMenuItemsInternal(juce::PopupMenu* m) override;
	void handleMenuSelectedID(int result) override;

	void showEditWindowInternal() override;
	void showEditRangeWindowInternal() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(P2DUI)
};