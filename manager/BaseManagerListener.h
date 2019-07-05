#pragma once

template<class T>
class  BaseManagerListener
{
public:
	/** Destructor. */
	virtual ~BaseManagerListener() {}
	virtual void itemAdded(T*) {}
	virtual void itemsAdded(Array<T*>) {}
	virtual void itemRemoved(T*) {}
	virtual void itemsRemoved(Array<T*>) {}
	virtual void itemsReordered() {}
};