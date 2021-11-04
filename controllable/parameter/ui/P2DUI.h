/*
  ==============================================================================

	P2DUI.h
	Created: 25 Oct 2016 11:46:46am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class P2DUI : 
	public ParameterUI,
	public Timer
{

public:
	P2DUI(Point2DParameter* parameter = nullptr);
	virtual ~P2DUI();

	Point2DParameter* p2d;

	var mouseDownNormalizedValue;
	var mouseDownValue;
	bool shouldRepaint;
	int updateRate;

	Rectangle<float> canvasRect;

	void mouseDownInternal(const MouseEvent&) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUpInternal(const MouseEvent&) override;

	void visibilityChanged() override;

	void paint(Graphics& g) override;
	void resized() override;

	virtual void valueChanged(const var& v) override;
	virtual void rangeChanged(Parameter* p) override;

	void addPopupMenuItemsInternal(PopupMenu* m) override;
	void handleMenuSelectedID(int result) override;

	void showEditWindowInternal() override;
	void showEditRangeWindowInternal() override;

	virtual void timerCallback() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(P2DUI)
};