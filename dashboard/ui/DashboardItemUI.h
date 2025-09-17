/*
  ==============================================================================

	DashboardUI.h
	Created: 19 Apr 2017 11:06:43pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemUI :
	public ItemMinimalUI<DashboardItem>,
	public Parameter::AsyncListener,
	public juce::SettableTooltipClient
{
public:
	DashboardItemUI(DashboardItem* item);
	~DashboardItemUI();

	virtual void paint(juce::Graphics& g) override;
	virtual void paintOverChildren(juce::Graphics& g) override;
	virtual void resized() override;
	virtual void resizedDashboardItemInternal() {}

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseEnter(const juce::MouseEvent& e) override;
	virtual void mouseDrag(const juce::MouseEvent& e) override;
	virtual void mouseUp(const juce::MouseEvent& e) override;
	virtual bool keyPressed(const juce::KeyPress& e) override;

	virtual void updateEditMode();
	virtual void updateEditModeInternal(bool editMode) {}

	ItemUI<DashboardItem>::ItemResizerComponent resizer;

	virtual void newMessage(const Parameter::ParameterEvent& e) override;

};