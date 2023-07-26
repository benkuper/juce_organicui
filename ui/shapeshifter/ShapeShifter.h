/*
  ==============================================================================

    ShapeShifter.h
    Created: 2 May 2016 6:33:16pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once


class ShapeShifterContainer;

class ShapeShifter : public juce::Component
{
public :
	enum Type {PANEL, CONTAINER};

	ShapeShifter(Type _type);
	virtual ~ShapeShifter();

	ShapeShifterContainer * parentContainer;

	Type shifterType;

	int preferredWidth;
	int preferredHeight;
	void setPreferredWidth(int newWidth);
	void setPreferredHeight(int newHeight);

	virtual int getPreferredWidth();
	virtual int getPreferredHeight();

	bool isDetached();
	void setParentContainer(ShapeShifterContainer * _parent);

	virtual bool isFlexible();

	virtual juce::var getCurrentLayout();
	virtual void loadLayout(juce::var layout);
	virtual void loadLayoutInternal(juce::var /*layout*/) {}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifter)
};