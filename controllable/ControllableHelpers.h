/*
  ==============================================================================

    ControllableHelpers.h
    Created: 12 May 2016 4:21:18pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ControllableChooserPopupMenu : 
	public PopupMenu
{
public:
	ControllableChooserPopupMenu(ControllableContainer* rootContainer, 
		int indexOffset = 0, int maxSearchLevel = -1, 
		const StringArray & typesFilter = StringArray(), const StringArray & excludeTypeFilters = StringArray(), std::function<bool(Controllable *)> filterFunc = nullptr);
	virtual ~ControllableChooserPopupMenu();

	int indexOffset;
	int maxDefaultSearchLevel;
	StringArray typesFilter;
	StringArray excludeTypesFilter;
	std::function<bool(Controllable*)> filterFunc;

	Array<Controllable *> controllableList;
	void populateMenu(PopupMenu *subMenu, ControllableContainer * container, int &currentId, int currentLevel = 0);

	void showAndGetControllable(std::function<void(Controllable *)> returnFunc);
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
	public PopupMenu
{
public:
	ContainerChooserPopupMenu(ControllableContainer * rootContainer, int indexOffset = 0, int maxSearchLevel = -1, std::function<bool(ControllableContainer *)> typeCheckFunc = nullptr, const StringArray& typesFilter = StringArray(), const StringArray& excludeTypeFilters = StringArray(),bool allowSelectAtAnylevel = false);
	virtual ~ContainerChooserPopupMenu();

	int indexOffset;
	int maxDefaultSearchLevel;
	std::function<bool(ControllableContainer *)> typeCheckFunc;
	StringArray typesFilter;
	StringArray excludeTypesFilter;
	bool allowSelectAtAnyLevel;

	Array<ControllableContainer *> containerList;
	void populateMenu(PopupMenu *subMenu, ControllableContainer * container, int &currentId, int currentLevel = 0);

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