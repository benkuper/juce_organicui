#pragma once

class BaseItem;

template<class T>
class ManagerTListener
{
public:
	//static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	//static_assert(std::is_base_of<ItemGroup<T>, G>::value, "G must be derived from ItemGroup<T>");
	//static_assert(std::is_base_of<BaseItem, G>::value, "G must be derived from BaseItem");

	/** Destructor. */
	virtual ~ManagerTListener() {}
	virtual void itemAdded(BaseItem*) {}
	virtual void itemsAdded(juce::Array<BaseItem*>) {}
	virtual void itemRemoved(BaseItem*) {}
	virtual void itemsRemoved(juce::Array<BaseItem*>) {}
	virtual void itemsReordered() {}
};


template<class T>
class ManagerEvent
{
public:
	enum Type
	{
		ITEM_ADDED,
		ITEM_REMOVED,
		ITEMS_ADDED,
		ITEMS_REMOVED,
		ITEMS_REORDERED,
		MANAGER_CLEARED,
		NEEDS_UI_UPDATE
	};

	ManagerEvent(Type type, juce::Array<BaseItem*> items = juce::Array<BaseItem*>(), bool fromChildGroup = false) :
		type(type),
		fromChildGroup(fromChildGroup)
	{
		this->items.addArray(items);
	}

	Type type;
	bool fromChildGroup;

	~ManagerEvent() {}

	bool isGroup() const {
		if (this->items.isEmpty()) return false;
		if (this->items.getFirst().wasObjectDeleted()) return false;
		return this->items.getFirst()->isGroup;
	}

	juce::Array<BaseItem*> getItems() const
	{
		juce::Array<BaseItem*> result;
		for (auto& i : items)
		{
			if (i == nullptr) continue;
			if (i.wasObjectDeleted()) continue;
			result.add(i);
		}
		return result;
	}
};