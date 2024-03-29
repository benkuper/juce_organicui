#pragma once

template<typename T>
class  BaseManagerListener
{
public:
	/** Destructor. */
	virtual ~BaseManagerListener() {}
	virtual void itemAdded(T*) {}
	virtual void itemsAdded(juce::Array<T*>) {}
	virtual void itemRemoved(T*) {}
	virtual void itemsRemoved(juce::Array<T*>) {}
	virtual void itemsReordered() {}
};
