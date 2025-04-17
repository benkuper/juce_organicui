/*
  ==============================================================================

	Manager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class BaseManager :
	public EnablingControllableContainer,
	public BaseItemListener
{
public:
	BaseManager(const juce::String& name);
	virtual ~BaseManager();

	bool canHaveGroups;

	int getItemIndex(BaseItem* item) const;
	int getItemIndex(const juce::String& name, bool searchNiceNameToo = false) const;

	bool hasItems(bool includeGroups = false, bool recursive = false) const;
	int getNumItems(bool includeGroups = false, bool recursive = false) const;
	bool hasItem(BaseItem* item, bool recursive = false) const;


protected:
	juce::OwnedArray<BaseItem, juce::CriticalSection> baseItems;
};