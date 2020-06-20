/*
  ==============================================================================

    InspectableContentComponent.h
    Created: 9 May 2016 6:51:16pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../InspectableContent.h"

class InspectableContentComponent : 
	public InspectableContent,
	public Inspectable::AsyncListener,
	public Component
{
public:
	InspectableContentComponent(Inspectable * inspectable);
	virtual ~InspectableContentComponent();

	bool repaintOnSelectionChanged;

	float rounderCornerSize;
	bool autoDrawContourWhenSelected;
	bool highlightLinkedInspectablesOnOver;

	Colour selectionContourColor;

	bool autoSelectWithChildRespect;
	Colour highlightColor;

	bool bringToFrontOnSelect;

	virtual void setInspectable(Inspectable* i) override;


	void mouseEnter(const MouseEvent &e) override;
	void mouseExit(const MouseEvent &e) override;

	void mouseDown(const MouseEvent &e) override;

	virtual void paintOverChildren(Graphics &g) override;

	virtual juce::Rectangle<int> getMainBounds();
	virtual int getExtraWidth();
	virtual int getExtraHeight();

	void newMessage(const Inspectable::InspectableEvent &e) override;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectableContentComponent)
};
