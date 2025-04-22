#pragma once

class BaseItem;

template<class T> class ItemGroup;

template<class T, class G = ItemBaseGroup<T>>
class ManagerTListener
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	static_assert(std::is_base_of<ItemGroup<T>, G>::value, "G must be derived from ItemGroup<T>");
	static_assert(std::is_base_of<BaseItem, G>::value, "G must be derived from BaseItem");

	/** Destructor. */
	virtual ~ManagerTListener() {}
	virtual void itemAdded(T*) {}
	virtual void itemsAdded(juce::Array<T*>) {}
	virtual void itemRemoved(T*) {}
	virtual void itemsRemoved(juce::Array<T*>) {}
	virtual void itemsReordered() {}

	virtual void groupAdded(G*) {}
	virtual void groupsAdded(juce::Array<G*>) {}
	virtual void groupRemoved(G*) {}
	virtual void groupsRemoved(juce::Array<G*>) {}
};
