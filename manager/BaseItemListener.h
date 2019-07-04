#pragma once

class BaseItem;

class  BaseItemListener
{
public:
	/** Destructor. */
	virtual ~BaseItemListener() {}
	virtual void askForRemoveBaseItem(BaseItem*) {}
	virtual void askForDuplicateItem(BaseItem*) {}
	virtual void askForPaste() {}
	virtual void askForMoveBefore(BaseItem*) {}
	virtual void askForMoveAfter(BaseItem*) {}
	virtual void askForSelectAllItems() {}
	virtual void askForSelectPreviousItem(BaseItem*, bool addToSelection = false) {}
	virtual void askForSelectNextItem(BaseItem*, bool addToSelection = false) {}
};