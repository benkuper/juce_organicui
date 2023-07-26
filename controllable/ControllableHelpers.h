/*
  ==============================================================================

    ControllableHelpers.h
    Created: 12 May 2016 4:21:18pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ControllableChooserPopupMenu : 
	public juce::PopupMenu
{
public:
	ControllableChooserPopupMenu(ControllableContainer* rootContainer, 
		int indexOffset = 0, int maxSearchLevel = -1, 
		const juce::StringArray & typesFilter = juce::StringArray(), const juce::StringArray & excludeTypeFilters = juce::StringArray(), std::function<bool(Controllable *)> filterFunc = nullptr,  Controllable* currentSelection = nullptr);
	virtual ~ControllableChooserPopupMenu();

	int indexOffset;
	int maxDefaultSearchLevel;
	juce::StringArray typesFilter;
	juce::StringArray excludeTypesFilter;
	std::function<bool(Controllable*)> filterFunc;
	Controllable* currentSelection;

	juce::Array<Controllable *> controllableList;
	void populateMenu(juce::PopupMenu *subMenu, ControllableContainer * container, int &currentId, int currentLevel = 0);

	void showAndGetControllable(std::function<void(Controllable *)> returnFunc, bool deleteAfter = false);
	Controllable * getControllableForResult(int result);
};


class ContainerTypeChecker
{
public:
	template<class T>
	static bool checkType(ControllableContainer * cc)
	{
		return dynamic_cast<T *>(cc) != nullptr;
	}
};

class ContainerChooserPopupMenu :
	public juce::PopupMenu
{
public:
	ContainerChooserPopupMenu(ControllableContainer * rootContainer, int indexOffset = 0, int maxSearchLevel = -1, std::function<bool(ControllableContainer *)> typeCheckFunc = nullptr, const juce::StringArray& typesFilter = juce::StringArray(), const juce::StringArray& excludeTypeFilters = juce::StringArray(),bool allowSelectAtAnylevel = false, ControllableContainer* currentSelection = nullptr);

	virtual ~ContainerChooserPopupMenu();

	int indexOffset;
	int maxDefaultSearchLevel;
	std::function<bool(ControllableContainer *)> typeCheckFunc;
	juce::StringArray typesFilter;
	juce::StringArray excludeTypesFilter;
	bool allowSelectAtAnyLevel;
	ControllableContainer* currentSelection;

	juce::Array<ControllableContainer *> containerList;
	void populateMenu(juce::PopupMenu *subMenu, ControllableContainer * container, int &currentId, int currentLevel = 0);

	void showAndGetContainer(std::function<void(ControllableContainer *)> returnFunc);
	ControllableContainer * getContainerForResult(int result);
};


//Comparator class to sort controllable array by name
class ControllableComparator
{
public:
	ControllableComparator() {}
	virtual ~ControllableComparator() {}
	virtual int compareElements(Controllable* c1, Controllable* c2);
};



class ControllableParser
{
public:
	static void createControllablesFromJSONObject(juce::var data, ControllableContainer* container);
	static void createControllableFromJSONObject(juce::StringRef name, juce::var data, ControllableContainer *container);
};