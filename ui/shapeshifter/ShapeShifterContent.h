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
	ShapeShifterContent(Component * contentComponent, const String &_contentName);
	virtual ~ShapeShifterContent();

	Component * contentComponent;
	String contentName;

	bool contentIsFlexible;

	bool contentIsShown;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterContent)
};

//Helper class if child class doesn't need to inherit a subclass of Component
class ShapeShifterContentComponent :
	public Component,
	public ShapeShifterContent
{
public:
	ShapeShifterContentComponent(const String &contentName);

	String helpID;

	void mouseEnter(const MouseEvent &e) override;
	void mouseExit(const MouseEvent &e) override;
};