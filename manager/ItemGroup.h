/*
  ==============================================================================

    BaseGroup.h
    Created: 15 Apr 2025 2:07:26pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BaseItemGroup
{
public:
	BaseItemGroup(BaseManager* _manager, const juce::String& name = "Group")
	{
		baseManager.reset(_manager);
	}

	virtual ~BaseItemGroup() {}

	std::unique_ptr<BaseManager> baseManager;
};

template<class T>
class ItemGroup :
	public BaseItemGroup
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	ItemGroup(Manager<T>* _manager, const juce::String& name = "Group") :
		BaseItemGroup(_manager, name)
	{
	}

	virtual ~ItemGroup() {}

	std::unique_ptr<Manager<T>> manager;
};


template<class T>
class ItemBaseGroup :
	public ItemGroup<T>,
	public BaseItem
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	ItemBaseGroup(Manager<T>* _manager, const juce::String& name = "Group") :
		ItemGroup<T>(_manager, name),
		BaseItem(name, true, true, true)
	{
	}

	virtual ~ItemBaseGroup() {}
};