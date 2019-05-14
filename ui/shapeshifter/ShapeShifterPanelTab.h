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
	public Component,
	public Button::Listener
{
public:
	ShapeShifterPanelTab(ShapeShifterContent * _content);
	~ShapeShifterPanelTab();

	ShapeShifterContent * content;
	Label panelLabel;
	ScopedPointer<ImageButton> closePanelBT;

	bool selected;
	void setSelected(bool value);

	void paint(Graphics &g) override;
	void resized()override;

	int getLabelWidth();

	void buttonClicked(Button * b) override;

	//Listener
	class TabListener
	{
	public:
		virtual ~TabListener() {};
		virtual void askForRemoveTab(ShapeShifterPanelTab *) {};
	};

	ListenerList<TabListener> tabListeners;
	void addShapeShifterTabListener(TabListener* newListener) { tabListeners.add(newListener); }
	void removeShapeShifterTabListener(TabListener* listener) { tabListeners.remove(listener); }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterPanelTab)
};