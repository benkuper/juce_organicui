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

	juce::String itemDataType;
	bool userCanAddItemsManually;
	bool selectItemWhenCreated;
	bool autoReorderOnAdd;
	bool isManipulatingMultipleItems;
	juce::Point<float> clipboardCopyOffset;

	//ui
	juce::Point<int> viewOffset; //in pixels, viewOffset of 0 means zeroPos is at the center of the window
	//interaction
	float viewZoom;

	//editor
	bool showItemsInEditor;
	// 
	//grid
	BoolParameter* snapGridMode;
	BoolParameter* showSnapGrid;
	IntParameter* snapGridSize;

	void setHasGridOptions(bool hasGridOptions);

	int getItemIndex(BaseItem* item) const;
	int getItemIndex(const juce::String& name, bool searchNiceNameToo = false) const;

	bool hasItems(bool includeGroups = false, bool recursive = false) const;
	int getNumItems(bool includeGroups = false, bool recursive = false) const;
	bool hasItem(BaseItem* item, bool recursive = false) const;

	class ManagerItemComparator
	{
	public:
		ManagerItemComparator(BaseManager* manager);

		BaseManager* m;

		std::function<int(BaseItem*, BaseItem*)> compareFunc;
		int compareElements(BaseItem* i1, BaseItem* i2);

	};

	ManagerItemComparator comparator;


protected:
	juce::OwnedArray<BaseItem, juce::CriticalSection> baseItems;
};