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
	public juce::Component
{
public:
	InspectableContentComponent(Inspectable * inspectable);
	virtual ~InspectableContentComponent();

	bool repaintOnSelectionChanged;

	float rounderCornerSize;
	bool autoDrawContourWhenSelected;
	bool highlightLinkedInspectablesOnOver;

	juce::Colour selectionContourColor;

	bool autoSelectWithChildRespect;
	juce::Colour highlightColor;

	bool bringToFrontOnSelect;

	virtual void setInspectable(Inspectable* i) override;


	void mouseEnter(const juce::MouseEvent &e) override;
	void mouseExit(const juce::MouseEvent &e) override;

	void mouseDown(const juce::MouseEvent &e) override;

	virtual void selectToThis();

	virtual void paintOverChildren(juce::Graphics &g) override;

	virtual juce::Rectangle<int> getMainBounds();
	virtual int getExtraWidth();
	virtual int getExtraHeight();

	virtual void newMessage(const Inspectable::InspectableEvent &e) override;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectableContentComponent)
};
