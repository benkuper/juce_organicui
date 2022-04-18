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
	BaseItemEditor(Array<BaseItem *> bi, bool isRoot);
	~BaseItemEditor();

	Array<BaseItem *> items;
	BaseItem* item;

	bool isFirst;
	bool isLast;

	std::unique_ptr<ImageButton> removeBT;
	std::unique_ptr<ImageButton> duplicateBT;
	std::unique_ptr<ImageButton> upBT;
	std::unique_ptr<ImageButton> downBT;

	void setIsFirst(bool value);
	void setIsLast(bool value);

	virtual void resizedInternalHeader(juce::Rectangle<int> & r) override;
	virtual void resizedInternalHeaderItemInternal(juce::Rectangle<int> &) {}
	virtual void buttonClicked(Button *b) override;
};