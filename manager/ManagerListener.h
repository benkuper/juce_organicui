#pragma once

class BaseItem;

template<class T, class G>
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
