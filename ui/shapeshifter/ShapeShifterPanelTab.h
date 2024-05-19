/*
  ==============================================================================

    ShapeShifterPanelTab.h
    Created: 3 May 2016 12:43:01pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "ShapeShifterContent.h"//keep
class ShapeShifterPanelTab :
	public juce::Component,
	public juce::Button::Listener,
	public ShapeShifterContent::AsyncListener
{
public:
	ShapeShifterPanelTab(ShapeShifterContent * _content);
	~ShapeShifterPanelTab();

	ShapeShifterContent * content;
	juce::Label panelLabel;
	std::unique_ptr<juce::ImageButton> closePanelBT;

	bool selected;
	void setSelected(bool value);

	void clear();

	void paint(juce::Graphics &g) override;
	void resized()override;

	int getLabelWidth();

	void buttonClicked(juce::Button * b) override;

	void mouseDoubleClick(const juce::MouseEvent& e) override;

	void newMessage(const ShapeShifterContent::ShapeShifterContentEvent& e) override;

	//Listener
	class TabListener
	{
	public:
		virtual ~TabListener() {};
		virtual void askForRemoveTab(ShapeShifterPanelTab *) {};
		virtual void nameChanged(ShapeShifterPanelTab *) {};
	};

	juce::ListenerList<TabListener> tabListeners;
	void addShapeShifterTabListener(TabListener* newListener) { tabListeners.add(newListener); }
	void removeShapeShifterTabListener(TabListener* listener) { tabListeners.remove(listener); }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterPanelTab)
};