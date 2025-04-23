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
		baseManager = _manager;
	}

	virtual ~BaseItemGroup() {}

	BaseManager* baseManager;
};

template<class T>
class ItemGroup :
	public BaseItemGroup
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	ItemGroup(Manager<T>* _manager, const juce::String& name = "Group") :
		BaseItemGroup(_manager, name),
		manager(_manager)
	{
	}

	virtual ~ItemGroup() {}

	Manager<T>* manager;
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
		this->addChildControllableContainer(this->manager, true, 0);
	}

	virtual void clearItem() override
	{
		BaseItem::clearItem();
		this->manager->clear();
	}

	virtual ~ItemBaseGroup() {}
};