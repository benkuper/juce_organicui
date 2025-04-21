/*
  ==============================================================================

	Manager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once




class BaseItemGroup;

class BaseManager :
	public EnablingControllableContainer,
	public BaseItemListener
{
public:
	BaseManager(const juce::String& name, bool canHaveGroups = false);
	virtual ~BaseManager() {}

	bool canHaveGroups;
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

	juce::OwnedArray<BaseItem, juce::CriticalSection> baseItems;

	juce::Array<BaseItem*> getBaseItems(bool recursive = true, bool includeDisabled = true, bool includeGroups = true) const;
	void callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func);
};