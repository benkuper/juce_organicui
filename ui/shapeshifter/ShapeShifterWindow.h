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
	public juce::ResizableWindow,
	public ShapeShifterPanel::Listener,
	public juce::Button::Listener
{
public:
	ShapeShifterWindow(ShapeShifterPanel* _panel, juce::Rectangle<int> bounds);
	~ShapeShifterWindow();


	enum DragMode { NONE, TAB, PANEL };

	ShapeShifterPanel* panel;
	std::unique_ptr<juce::ImageButton> pinBT;

	void paintOverChildren(juce::Graphics& g)override;
	void resized() override;

	juce::ComponentDragger dragger;
	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	DragMode dragMode;

	void clear();

	void userTriedToCloseWindow() override;

	virtual juce::var getCurrentLayout();

	void buttonClicked(juce::Button* b) override;

	void panelEmptied(ShapeShifterPanel*) override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterWindow)
};