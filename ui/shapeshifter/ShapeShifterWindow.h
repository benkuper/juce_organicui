/*
  ==============================================================================

	ShapeShifterWindow.h
	Created: 2 May 2016 4:10:48pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "ShapeShifterPanel.h"

class ShapeShifterWindow :
	public ResizableWindow,
	public ShapeShifterPanel::Listener,
	public Button::Listener
{
public:
	ShapeShifterWindow(ShapeShifterPanel* _panel, juce::Rectangle<int> bounds);
	~ShapeShifterWindow();


	enum DragMode { NONE, TAB, PANEL };

	ShapeShifterPanel* panel;
	std::unique_ptr<ImageButton> pinBT;

	void paintOverChildren(Graphics& g)override;
	void resized() override;

	ComponentDragger dragger;
	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;

	DragMode dragMode;

	void clear();

	void userTriedToCloseWindow() override;

	virtual var getCurrentLayout();

	void buttonClicked(Button* b) override;

	void panelEmptied(ShapeShifterPanel*) override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterWindow)
};