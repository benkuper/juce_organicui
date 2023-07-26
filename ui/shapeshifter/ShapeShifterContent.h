/*
  ==============================================================================

    ShapeShifterContent.h
    Created: 3 May 2016 10:17:41am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ShapeShifterContent
{
public:
	ShapeShifterContent(juce::Component * contentComponent, const juce::String &_contentName);
	virtual ~ShapeShifterContent();

	juce::Component * contentComponent;
	juce::String contentName;

	bool contentIsFlexible;

	bool contentIsShown;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterContent)
};

//Helper class if child class doesn't need to inherit a subclass of Component
class ShapeShifterContentComponent :
	public juce::Component,
	public ShapeShifterContent
{
public:
	ShapeShifterContentComponent(const juce::String &contentName);

	juce::String helpID;

	void mouseEnter(const juce::MouseEvent &e) override;
	void mouseExit(const juce::MouseEvent &e) override;
};