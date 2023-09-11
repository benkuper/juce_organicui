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
	BaseItemEditor(juce::Array<BaseItem *> bi, bool isRoot);
	~BaseItemEditor();

	juce::Array<BaseItem *> items;
	BaseItem* item;

	//bool isFirst;
	//bool isLast;


	std::unique_ptr<juce::ImageButton> removeBT;
	std::unique_ptr<juce::ImageButton> duplicateBT;
	//std::unique_ptr<ImageButton> upBT;
	//std::unique_ptr<ImageButton> downBT;
	std::unique_ptr<ColorParameterUI> itemColorUI;

	//void setIsFirst(bool value);
	//void setIsLast(bool value);

	virtual void resizedInternalHeader(juce::Rectangle<int> & r) override;
	virtual void resizedInternalHeaderItemInternal(juce::Rectangle<int> &) {}
	virtual void buttonClicked(juce::Button *b) override;

	virtual void setDragDetails(juce::var &details) override;
};