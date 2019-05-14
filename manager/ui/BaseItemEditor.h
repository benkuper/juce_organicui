/*
  ==============================================================================

    BaseItemEditor.h
    Created: 18 Jan 2017 2:23:31pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class BaseItemEditor :
	public EnablingControllableContainerEditor
{
public:
	BaseItemEditor(BaseItem * bi, bool isRoot);
	~BaseItemEditor();

	BaseItem * item;

	bool isFirst;
	bool isLast;

	ScopedPointer<ImageButton> removeBT;
	ScopedPointer<ImageButton> duplicateBT;
	ScopedPointer<ImageButton> upBT;
	ScopedPointer<ImageButton> downBT;

	void setIsFirst(bool value);
	void setIsLast(bool value);

	virtual void resizedInternalHeader(juce::Rectangle<int> & r) override;
	virtual void resizedInternalHeaderItemInternal(juce::Rectangle<int> &) {}
	virtual void buttonClicked(Button *b) override;

};