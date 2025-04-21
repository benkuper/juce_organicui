#pragma once

class BaseItem;

template<typename T>
class ManagerTListener
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	/** Destructor. */
	virtual ~ManagerTListener() {}
	virtual void itemAdded(T*) {}
	virtual void itemsAdded(juce::Array<T*>) {}
	virtual void itemRemoved(T*) {}
	virtual void itemsRemoved(juce::Array<T*>) {}
	virtual void itemsReordered() {}
};
