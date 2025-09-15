/*
  ==============================================================================

	Manager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class M> class ItemGroup;

template <class T>
class Manager :
	public EnablingControllableContainer
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	Manager(const juce::String& name = "Items", bool canHaveGroups = false);
	virtual ~Manager() {}

	bool canHaveGroups;
	bool userCanAddItemsManually;
	bool selectItemWhenCreated;
	bool isManipulatingMultipleItems;
	juce::Point<float> clipboardCopyOffset;

	//ui
	juce::Point<int> viewOffset; //in pixels, viewOffset of 0 means zeroPos is at the center of the window
	float viewZoom;
	bool showItemsInEditor;
	BoolParameter* snapGridMode;
	BoolParameter* showSnapGrid;
	IntParameter* snapGridSize;

	juce::String itemDataType;


	//Factory
	Factory* managerFactory;

	virtual void clear() override;

	//Adding
	bool canAddItemOfType(const juce::String& typeToCheck);

	virtual T* createItem(); //to override if special constructor to use
	BaseItem* addItem(BaseItem* item = nullptr, juce::var data = juce::var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data
	juce::Array<BaseItem*> addItems(juce::Array<BaseItem*> items, juce::var data = juce::var(), bool addToUndo = true, bool notify = true);

	virtual BaseItem* createItemFromData(juce::var data);
	virtual BaseItem* addItemFromData(juce::var data, bool addToUndo = true);
	virtual juce::Array<BaseItem*> addItemsFromData(juce::var data, bool addToUndo = true);
	virtual juce::Array<BaseItem*> addItemsFromClipboard(bool showWarning = true);

	//Removing
	BaseItem* removeItem(BaseItem* item, bool addToUndo = true, bool notify = true, bool returnItem = false);
	void removeItems(juce::Array<BaseItem*> items, bool addToUndo = true, bool notify = true);

	//Undo
	virtual juce::UndoableAction* getAddItemUndoableAction(BaseItem* item = nullptr, juce::var data = juce::var());
	virtual juce::UndoableAction* getAddItemsUndoableAction(juce::Array<BaseItem*> item = nullptr, juce::var data = juce::var());
	virtual juce::Array<juce::UndoableAction*> getRemoveItemUndoableAction(BaseItem* item);
	virtual juce::Array<juce::UndoableAction*> getRemoveItemsUndoableAction(juce::Array<BaseItem*> items);


	//to  for specific handling like adding custom listeners, etc.
	virtual void addItemInternal(BaseItem*, juce::var data) {}
	virtual void addItemsInternal(juce::Array<BaseItem*>, juce::var data) {}
	virtual void removeItemInternal(BaseItem*) {}
	virtual void removeItemsInternal(juce::Array<BaseItem*>) {}


	//Ordering
	virtual void reorderItems() {}
	virtual void setItemIndex(BaseItem* item, int newIndex, bool addToUndo = true);
	virtual juce::Array<juce::UndoableAction*> getSetItemIndexUndoableAction(BaseItem* item, int newIndex);


	//Helpers
	virtual int getNumItems(bool recursive = true, bool includeDisabled = true, bool includeGroups = true) const;
	virtual bool hasNoItems(bool recursive = true, bool includeDisabled = true, bool includeGroups = true) const;
	virtual juce::Array<BaseItem*> getAllItems(bool recursive = true, bool includeDisabled = true, bool includeGroups = true) const;
	virtual juce::Array<T*> getItems(bool includeDisabled = true, bool recursive = true);
	virtual juce::Array<ItemGroup<T>*> getGroups(bool includeDisabled = true, bool recursive = true);


	virtual juce::String getItemPath(BaseItem* item);
	virtual BaseItem* getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo = false, bool searchWithLowerCaseIfNotFound = true, bool recursive = true);

	virtual BaseItem* getItemWithPath(const juce::String& relativePath);

	int getItemIndex(BaseItem* item) const;
	BaseItem* getItemAt(int index) const;
	bool hasItem(BaseItem* item) const;

	template<class IType>
	juce::Array<IType*> getItemsWithType(bool recursive = true);
	juce::Array<T*> getAsItems(juce::Array<BaseItem*> _items) const;
	juce::Array<ItemGroup<T>*> getAsGroups(juce::Array<BaseItem*> _items) const;
	T* getAsItem(BaseItem* baseItem) const;
	ItemGroup<T>* getAsGroup(BaseItem* baseItem) const;

	void callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func);
	void callFunctionOnItemsOnly(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(T*)> func);


	//generic overrides
	void onContainerParameterChanged(Parameter* p) override;


	juce::PopupMenu getItemsMenu(int startID);
	T* getItemForMenuResultID(int id, int startID);

	T* getFirstSelectedItem();


	//Save / Load / Data

	virtual juce::var getExportSelectionData();
	virtual void getRemoteControlDataInternal(juce::var& data) override;
	virtual void handleAddFromRemoteControl(juce::var data) override;


	virtual juce::var getJSONData(bool includeNonOverriden = false) override;
	virtual void loadItemsData(juce::var data);


	//ui
	void setHasGridOptions(bool hasGridOptions);


	typedef ManagerTListener<T> ManagerListener;

	juce::ListenerList<ManagerListener> managerListeners;
	juce::ListenerList<ManagerListener> recursiveManagerListeners;
	void addManagerListener(ManagerListener* newListener, bool recursive = true) {
		if (isBeingDestroyed) return;
		managerListeners.add(newListener);
		if (canHaveGroups && recursive) recursiveManagerListeners.add(newListener);
	}

	void removeManagerListener(ManagerListener* listener) {
		if (isBeingDestroyed) return;
		managerListeners.remove(listener);
		if (canHaveGroups) recursiveManagerListeners.remove(listener);
	}


	InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;

	class ManagerItemComparator
	{
	public:
		ManagerItemComparator(Manager* manager);

		Manager* m;

		std::function<int(T*, T*)> compareFunc;
		int compareElements(ControllableContainer* i1, ControllableContainer* i2);

	};

	ManagerItemComparator comparator;


	//Scripting
	static juce::var addItemFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var removeItemFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var removeAllItemsFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getItemsFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getItemWithNameFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getItemAtFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getItemIndexFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getItemBeforeFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getItemAfterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var reorderItemsFromScript(const juce::var::NativeFunctionArgs& args);
	juce::String getScriptTargetString() override;

	
	typedef typename ManagerEvent<T>::Type ManagerEventType;

	virtual void notifyAsync(ManagerEventType type, juce::Array<BaseItem*> items = juce::Array<BaseItem*>(), bool fromChildGroup = false);

	using ManagerNotifier = QueuedNotifier<ManagerEvent<T>>;
	ManagerNotifier managerNotifier;
	ManagerNotifier recursiveManagerNotifier;
	typedef typename QueuedNotifier<ManagerEvent<T>>::Listener AsyncListener;

	void addAsyncManagerListener(AsyncListener* newListener, bool recursive = true) {
		managerNotifier.addListener(newListener);
		if (canHaveGroups && recursive) recursiveManagerNotifier.addListener(newListener);
	}

	void addAsyncCoalescedManagerListener(AsyncListener* newListener, bool recursive = true)
	{
		managerNotifier.addCoalescedListener(newListener);
		if (canHaveGroups && recursive) recursiveManagerNotifier.addCoalescedListener(newListener);
	}

	void removeAsyncManagerListener(AsyncListener* listener) {
		managerNotifier.removeListener(listener);
		if (canHaveGroups) recursiveManagerNotifier.removeListener(listener);
	}



private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Manager);

	juce::Array<BaseItem*, juce::CriticalSection> items;

	void askForPaste();
	void askForSelectAllItems(bool addToSelection = false);
	void askForRemoveBaseItem(BaseItem* item);
	void askForDuplicateItem(BaseItem* item);
	void askForMoveBefore(BaseItem*);
	void askForMoveAfter(BaseItem*);
	void askForSelectPreviousItem(BaseItem* item, bool addToSelection = false);
	void askForSelectNextItem(BaseItem* item, bool addToSelection = false);



	virtual void loadJSONDataInternal(juce::var data) override;
	virtual void loadJSONDataManagerInternal(juce::var data);


	//notify, to override by templated classes
	virtual void notifyItemAdded(BaseItem* item, bool fromChildGroup = false);
	virtual void notifyItemsAdded(juce::Array<BaseItem*> items, bool fromChildGroup = false);
	virtual void notifyItemRemoved(BaseItem* item, bool fromChildGroup = false);
	virtual void notifyItemsRemoved(juce::Array<BaseItem*> items, bool fromChildGroup = false);
	virtual void notifyItemsReordered(bool fromChildGroup = false);

	//UNDO MANAGER
	class ManagerBaseAction :
		public juce::UndoableAction
	{
	public:
		ManagerBaseAction(Manager* manager, juce::var _data = juce::var());

		juce::String managerControlAddress;
		juce::var data;
		juce::WeakReference<Inspectable> managerRef;

		Manager* getManager();
	};

	class ItemBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemBaseAction(Manager* m, BaseItem* i, juce::var data = juce::var());

		juce::WeakReference<BaseItem> baseItem;
		juce::String itemShortName;
		int itemIndex;

		BaseItem* getItem();
	};

	class AddItemAction :
		public ItemBaseAction
	{
	public:
		AddItemAction(Manager* m, BaseItem* i, juce::var data = juce::var());

		bool perform() override;
		bool undo() override;
	};

	class RemoveItemAction :
		public ItemBaseAction
	{
	public:
		RemoveItemAction(Manager* m, BaseItem* i, juce::var data = juce::var());

		bool perform() override;
		bool undo() override;
	};

	class MoveItemAction :
		public ItemBaseAction
	{
	public:
		MoveItemAction(Manager* m, BaseItem* i, int prevIndex, int newIndex);

		int prevIndex;
		int newIndex;

		bool perform() override;
		bool undo() override;
	};

	//Multi add/remove items actions
	class ItemsBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemsBaseAction(Manager* m, juce::Array<BaseItem*> iList, juce::var data = juce::var());

		juce::Array<juce::WeakReference<BaseItem>> _items;
		juce::StringArray itemsShortName;

		juce::Array<BaseItem*> getItems();

	};

	class AddItemsAction :
		public ItemsBaseAction
	{
	public:
		AddItemsAction(Manager* m, juce::Array<BaseItem*> iList, juce::var data = juce::var());

		int startIndex;
		bool perform() override;
		bool undo() override;
	};


	class RemoveItemsAction :
		public ItemsBaseAction
	{
	public:
		RemoveItemsAction(Manager* m, juce::Array<BaseItem*> iList);

		bool perform() override;
		bool undo() override;
	};


};


template<class T>
Manager<T>::Manager(const juce::String& name, bool canHaveGroups) :
	EnablingControllableContainer(name, true),
	canHaveGroups(canHaveGroups),
	userCanAddItemsManually(true),
	selectItemWhenCreated(false),
	isManipulatingMultipleItems(false),
	clipboardCopyOffset(0, 0),
	viewOffset(0, 0),
	viewZoom(1.0f),
	showItemsInEditor(true),
	snapGridMode(nullptr),
	showSnapGrid(nullptr),
	snapGridSize(nullptr),
	managerFactory(nullptr),
	managerNotifier(50),
	recursiveManagerNotifier(50),
	comparator(this)
{
	canBeCopiedAndPasted = true;
	skipLabelInTarget = true; //by default manager label in targetParameter UI are not interesting
	nameCanBeChangedByUser = false;


	scriptObject.getDynamicObject()->setMethod("addItem", &Manager<T>::addItemFromScript);
	scriptObject.getDynamicObject()->setMethod("removeItem", &Manager<T>::removeItemFromScript);
	scriptObject.getDynamicObject()->setMethod("removeAll", &Manager<T>::removeAllItemsFromScript);
	scriptObject.getDynamicObject()->setMethod("getItems", &Manager<T>::getItemsFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemWithName", &Manager<T>::getItemWithNameFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemAt", &Manager<T>::getItemAtFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemIndex", &Manager<T>::getItemIndexFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemBefore", &Manager<T>::getItemBeforeFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemAfter", &Manager<T>::getItemAfterFromScript);
	scriptObject.getDynamicObject()->setMethod("reorderItems", &Manager<T>::reorderItemsFromScript);

}


template<class T>
void Manager<T>::clear()
{
	isClearing = true;
	//const ScopedLock lock(this->items.getLock());
	while (this->items.size() > 0) removeItem(this->items[0], false);
	isClearing = false;

#if ORGANICUI_USE_WEBSERVER
	if (notifyRemoteControlOnClear && !isCurrentlyLoadingData && isAttachedToRoot()) OSCRemoteControl::getInstance()->sendPathChangedFeedback(getControlAddress());
#endif
}

template<class T>
BaseItem* Manager<T>::addItem(BaseItem* item, juce::var data, bool addToUndo, bool notify)
{

	if (item == nullptr) return nullptr; //could not create here

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performAction("Add " + item->niceName, new AddItemAction(this, item, data));
			return item;
		}
	}
	item->addBaseItemListener(this);
	if (!data.isVoid()) item->loadJSONData(data);
	addChildControllableContainer(item, false, this->items.indexOf(item), notify);

	int targetIndex = data.getProperty("index", -1);
	if (targetIndex != -1) this->items.insert(targetIndex, item);
	else this->items.add(item);

	addItemManagerInternal(item, data);

	if (notify)
	{
		notifyItemAdded(item);
	}

	if (MessageManager::getInstance()->existsAndIsLockedByCurrentThread())
	{
		if (selectItemWhenCreated && !isCurrentlyLoadingData && !isManipulatingMultipleItems) item->selectThis();
	}

	return item;
}

template<class T>
juce::Array<BaseItem*> Manager<T>::addItems(juce::Array<BaseItem*> itemsToAdd, juce::var data, bool addToUndo, bool notify)
{
	bool curIsLoadingData = isCurrentlyLoadingData;
	bool curIsManipulatingMultipleItems = isManipulatingMultipleItems;

	isCurrentlyLoadingData = true;
	isManipulatingMultipleItems = true;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		AddItemsAction* a = new AddItemsAction(this, itemsToAdd, data);
		UndoMaster::getInstance()->performAction("Add " + juce::String(itemsToAdd.size()) + " items", a);
		isCurrentlyLoadingData = curIsLoadingData;
		isManipulatingMultipleItems = curIsManipulatingMultipleItems;
		return itemsToAdd;
	}

	for (int i = 0; i < itemsToAdd.size(); ++i)
	{
		addItem(itemsToAdd[i], data.isArray() ? data[i] : juce::var(), false, false);
	}

	notifyStructureChanged();

	reorderItems();
	isCurrentlyLoadingData = curIsLoadingData;
	isManipulatingMultipleItems = curIsManipulatingMultipleItems;

	if (selectItemWhenCreated && !isCurrentlyLoadingData)
	{
		juce::Array<Inspectable*> select;
		for (auto& i : itemsToAdd) select.add(i);
		selectionManager->selectInspectables(select);
	}

	addItemsManagerInternal(itemsToAdd, data);

	if (notify) {
		notifyItemsAdded(itemsToAdd);
	}

	return itemsToAdd;
}

//if data is not empty, load data
template<class T>
BaseItem* Manager<T>::addItemFromData(juce::var data, bool addToUndo)
{
	BaseItem* item = createItemFromData(data);
	if (item == nullptr) return nullptr;
	return addItem(item, data, addToUndo, true);
}

template<class T>
juce::Array<BaseItem*> Manager<T>::addItemsFromData(juce::var data, bool addToUndo)
{
	juce::Array<BaseItem*> itemsToAdd;

	juce::var itemsData; //avoid offset between items array and data array if items are skipped because they're null
	for (int i = 0; i < data.size(); i++)
	{
		if (BaseItem* item = createItemFromData(data[i]))
		{
			itemsToAdd.add(item);
			itemsData.append(data[i]);
		}
	}
	return addItems(itemsToAdd, itemsData, addToUndo);
}

template<class T>
juce::Array<BaseItem*> Manager<T>::addItemsFromClipboard(bool showWarning)
{
	if (!userCanAddItemsManually) return juce::Array<BaseItem*>();
	juce::String s = SystemClipboard::getTextFromClipboard();
	juce::var data = JSON::parse(s);
	if (data.isVoid()) return juce::Array<BaseItem*>();

	if (!data.hasProperty("itemType"))
	{
		juce::Array<BaseItem*> result;
		result.add(this->addItemFromData(data));
		return result;
	}

	juce::String t = data.getProperty("itemType", "");
	if (!canAddItemOfType(t))
	{
		if (showWarning) NLOGWARNING(niceName, "Can't paste data from clipboard : data is of wrong type (\"" + t + "\").");
		return juce::Array<BaseItem*>();
	}

	juce::var itemsData = data.getProperty("items", juce::var());
	int sIndex = this->items.indexOf(InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>());
	if (sIndex >= 0)
	{
		sIndex++;
		for (int i = 0; i < itemsData.size(); i++)
		{
			itemsData[i].getDynamicObject()->setProperty("index", sIndex++);
		}
	}

	juce::Array<BaseItem*> copiedItems = addItemsFromData(itemsData);

	if (!clipboardCopyOffset.isOrigin())
	{
		for (auto& i : copiedItems)
		{
			((BaseItem*)i)->viewUIPosition->setPoint(((BaseItem*)i)->viewUIPosition->getPoint() + clipboardCopyOffset);
		}
	}

	return copiedItems;
}


template<class T>
bool Manager<T>::canAddItemOfType(const juce::String& typeToCheck)
{
	if (typeToCheck == itemDataType) return true;
	return false;
}


template<class T>
void Manager<T>::loadItemsData(juce::var data)
{
	if (data == juce::var()) return;
	juce::Array<juce::var>* itemsData = data.getProperty("items", juce::var()).getArray();
	if (itemsData == nullptr) return;

	for (auto& td : *itemsData)
	{
		juce::String n = td.getProperty("niceName", "");
		BaseItem* i = getItemWithName(n, true);
		if (i != nullptr) i->loadJSONData(td);
	}
}

template<class T>
BaseItem* Manager<T>::removeItem(BaseItem* item, bool addToUndo, bool notify, bool returnItem)
{
	if (item == nullptr) return nullptr;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performActions("Remove " + item->getTypejuce::String(), getRemoveItemUndoableAction(item));
			return nullptr;
		}
	}

	this->items.removeObject(item, false);

	item->removeBaseItemListener(this);
	removeChildControllableContainer(item);

	removeItemManagerInternal(item);

	if (notify)
	{
		notifyItemRemoved(item);
	}

	if (returnItem) return item; //will need to delete !

	item->clearItem();
	delete item;

	return nullptr;
}

template<class T>
void Manager<T>::removeItems(juce::Array<BaseItem*> itemsToRemove, bool addToUndo, bool notify)
{
	isManipulatingMultipleItems = true;
	if (addToUndo)
	{
		juce::Array<juce::UndoableAction*> a = getRemoveItemsUndoableAction(itemsToRemove);
		UndoMaster::getInstance()->performActions("Remove " + juce::String(itemsToRemove.size()) + " items", a);
		isManipulatingMultipleItems = false;
		return;
	}

	juce::Array<BaseItem*> itemsRemoved;
	for (auto& i : itemsToRemove) itemsRemoved.add(removeItem(i, false, false, true));

	removeItemsManagerInternal(itemsRemoved);

	if (notify)
	{
		notifyItemsRemoved(itemsRemoved);
	}

	for (auto& i : itemsRemoved)
	{
		((BaseItem*)i)->clearItem();
		delete i;
	}

	isManipulatingMultipleItems = false;
}

template<class T>
juce::UndoableAction* Manager<T>::getAddItemUndoableAction(BaseItem* item, juce::var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	jassert(this->items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	return new AddItemAction(this, item, data);
}

template<class T>
juce::UndoableAction* Manager<T>::getAddItemsUndoableAction(juce::Array<BaseItem*> _items, juce::var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	if (_items.size() == 0) return nullptr;
	return new AddItemsAction(this, _items, data);
}

template<class T>
juce::Array<juce::UndoableAction*> Manager<T>::getRemoveItemUndoableAction(BaseItem* item)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	juce::Array<juce::UndoableAction*> a;
	a.add(new RemoveItemAction(this, item));
	return a;
}

template<class T>
juce::Array<juce::UndoableAction*> Manager<T>::getRemoveItemsUndoableAction(juce::Array<BaseItem*> _items)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	juce::Array<juce::UndoableAction*> a;
	a.add(new RemoveItemsAction(this, _items));
	return a;
}


template<class T>
void Manager<T>::setItemIndex(BaseItem* item, int newIndex, bool addToUndo)
{
	if (item == nullptr) return;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			BaseItem* bi = static_cast<BaseItem*>(item);
			UndoMaster::getInstance()->performActions("Move " + bi->getTypejuce::String(), getSetItemIndexUndoableAction(item, newIndex));
			return;
		}
	}


	newIndex = jlimit(0, this->items.size() - 1, newIndex);
	int index = this->items.indexOf(item);
	if (index == -1 || index == newIndex) return;

	//items.getLock().enter();
	this->items.move(index, newIndex);
	controllableContainers.move(index, newIndex);
	//items.getLock().exit();

	notifyItemsReordered();
}

template<class T>
juce::Array<juce::UndoableAction*> Manager<T>::getSetItemIndexUndoableAction(BaseItem* item, int newIndex)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	juce::Array<juce::UndoableAction*> a;
	a.add(new MoveItemAction(this, item, this->items.indexOf(item), newIndex));
	return a;
}

template<class T>
BaseItem* Manager<T>::getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo, bool searchWithLowerCaseIfNotFound, bool recursive)
{
	//const ScopedLock lock(this->items.getLock());

	juce::Array<BaseItem*> itemsToSearch = getAllItems(recursive, true, false);

	for (auto& t : itemsToSearch)
	{
		if (((BaseItem*)t)->shortName == itemShortName) return t;
		else if (searchNiceNameToo && ((BaseItem*)t)->niceName == itemShortName) return t;

	}

	if (searchWithLowerCaseIfNotFound)
	{
		for (auto& t : itemsToSearch)
		{
			if (((BaseItem*)t)->shortName.toLowerCase() == itemShortName.toLowerCase()) return t;
		}
	}

	return nullptr;
}

template<class T>
juce::String Manager<T>::getItemPath(BaseItem* item)
{
	return item->getControlAddress(this);
}

template<class T>
BaseItem* Manager<T>::getItemWithPath(const juce::String& relativePath)
{
	if (BaseItem* i = dynamic_cast<BaseItem*>(getControllableContainerForAddress(relativePath))) return i;
	return nullptr;
}

template<class T>
int Manager<T>::getItemIndex(BaseItem* item) const
{
	return items.indexOf(item);
}

template<class T>
BaseItem* Manager<T>::getItemAt(int index) const
{
	if (index < 0 || index >= items.size()) return nullptr;
	return items[index];
}

template<class T>
bool Manager<T>::hasItem(BaseItem* item) const
{
	return items.contains(item);
}


template<class T>
void Manager<T>::callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func)
{
	Array<BaseItem*> itemsToCall = getAllItems(recursive, includeDisabled, includeGroups);
	for (auto& i : itemsToCall) func(i);
}

template<class T>
void Manager<T>::callFunctionOnItemsOnly(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(T*)> func)
{
	Array<T*> itemsToCall = getItems(includeDisabled, recursive);
	for (auto& i : itemsToCall) func(i);
}

template<class T>
void Manager<T>::handleAddFromRemoteControl(juce::var data)
{
	if (!userCanAddItemsManually) return;
	addItemFromData(data);
}

template<class T>
void Manager<T>::askForRemoveBaseItem(BaseItem* item)
{
	removeItem(item);
}

template<class T>
void Manager<T>::askForDuplicateItem(BaseItem* item)
{
	if (!userCanAddItemsManually) return;
	juce::var data = item->getJSONData();
	data.getDynamicObject()->setProperty("index", this->items.indexOf(static_cast<BaseItem*>(item)) + 1);
	addItemFromData(data);
}

template<class T>
void Manager<T>::askForPaste()
{
	addItemsFromClipboard();
}

template<class T>
void Manager<T>::askForMoveBefore(BaseItem* item)
{
	setItemIndex(item, jmax(this->items.indexOf(item) - 1, 0));
	//	int index = this->items.indexOf(static_cast<T *>(i));
	//	if (index == 0) return;
	//	this->items.swap(index, index - 1);
	//	controllableContainers.swap(index, index - 1);
	//
	//	managerListeners.call(&ManagerListener::itemsReordered);
	//	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T>
void Manager<T>::askForMoveAfter(BaseItem* item)
{
	setItemIndex(item, jmin(this->items.indexOf(item) + 1, this->items.size() - 1));
	//int index = this->items.indexOf(static_cast<T *>(i));
	//if (index == this->items.size() -1) return;
	//this->items.swap(index, index + 1);
	//controllableContainers.swap(index, index+1);

	//managerListeners.call(&ManagerListener::itemsReordered);
	//managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T>
void Manager<T>::askForSelectAllItems(bool addToSelection)
{
	int numItems = this->items.size();
	if (!addToSelection) selectionManager->clearSelection(numItems == 0);
	else deselectThis(numItems == 0);

	if (numItems > 1) for (int i = 0; i < numItems; ++i) this->items[i]->selectThis(true, i == numItems - 1); //only notify on last
	else if (numItems > 0) this->items.getFirst()->selectThis(addToSelection, true);
}

template<class T>
void Manager<T>::askForSelectPreviousItem(BaseItem* item, bool addToSelection)
{
	int index = this->items.indexOf(item);
	if (index <= 0) return;
	this->items[index - 1]->selectThis(addToSelection);
}

template<class T>
void Manager<T>::askForSelectNextItem(BaseItem* item, bool addToSelection)
{
	int index = this->items.indexOf(item);
	if (index == -1 || index >= this->items.size() - 1) return;
	this->items[index + 1]->selectThis(addToSelection);
}

template<class T>
void Manager<T>::onContainerParameterChanged(Parameter* p)
{
	EnablingControllableContainer::onContainerParameterChanged(p);
	if (p == showSnapGrid || p == snapGridSize)
	{
		notifyAsync(ManagerEvent::NEEDS_UI_UPDATE);
	}
}

template<class T>
juce::var Manager<T>::getExportSelectionData()
{
	juce::var data;

	for (auto& t : this->items)
	{
		if (t->isSavable && t->isSelected) data.append(t->getJSONData());
	}

	return data;
}

template<class T>
juce::var Manager<T>::getJSONData(bool includeNonOverriden)
{
	juce::var data = ControllableContainer::getJSONData(includeNonOverriden);
	juce::var itemsData = juce::var();
	//this->items.getLock().enter();
	for (auto& t : this->items)
	{
		if (t->isSavable) itemsData.append(t->getJSONData(includeNonOverriden));
	}
	//this->items.getLock().exit();

	if (itemsData.size() > 0) data.getDynamicObject()->setProperty("items", itemsData);

	juce::var vData;
	vData.append(viewOffset.x);
	vData.append(viewOffset.y);
	data.getDynamicObject()->setProperty("viewOffset", vData);
	data.getDynamicObject()->setProperty("viewZoom", viewZoom);

	return data;
}
template<class T>
void Manager<T>::loadJSONDataInternal(juce::var data)
{
	clear();

	if (data.hasProperty("viewOffset"))
	{
		juce::var vData = data.getProperty("viewOffset", juce::var());
		viewOffset.setXY(vData[0], vData[1]);
	}

	viewZoom = data.getProperty("viewZoom", 1);

	loadJSONDataManagerInternal(data);
}
template<class T>
void Manager<T>::loadJSONDataManagerInternal(juce::var data)
{
	juce::var itemsData = data.getProperty("items", juce::var());
	if (itemsData.isVoid()) return;

	addItemsFromData(itemsData, false);
}

template<class T>
void Manager<T>::getRemoteControlDataInternal(juce::var& data)
{
	data.getDynamicObject()->setProperty("TYPE", "Manager");
	data.getDynamicObject()->setProperty("BASE_TYPE", itemDataType);

	juce::var extType = juce::var();
	if (managerFactory != nullptr)
	{
		for (auto& d : managerFactory->defs) extType.append(d->menuPath + "/" + d->type);
	}
	else extType.append(itemDataType);

	data.getDynamicObject()->setProperty("EXTENDED_TYPE", extType);
}

template<class T>
juce::String Manager<T>::getScriptTargetString()
{
	return "[" + niceName + " : Manager(" + itemDataType + ")]";
}

//ACTIONS
template<class T>
Manager<T>::ManagerBaseAction::ManagerBaseAction(Manager<T>* manager, juce::var _data) :
	managerControlAddress(manager->getControlAddress()),
	data(_data),
	managerRef(manager)
{
}

template<class T>
Manager<T>* Manager<T>::ManagerBaseAction::getManager() {
	if (managerRef != nullptr && !managerRef.wasObjectDeleted()) return dynamic_cast<Manager*>(managerRef.get());
	else if (Engine::mainEngine != nullptr)
	{
		ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(managerControlAddress, true);
		if (cc != nullptr) return dynamic_cast<Manager*>(cc);
	}

	return nullptr;
}

template<class T>
Manager<T>::ItemBaseAction::ItemBaseAction(Manager* m, BaseItem* i, juce::var data) :
	ManagerBaseAction(m, data),
	baseItem(i),
	itemIndex(0)
{
	if (baseItem != nullptr)
	{
		this->itemShortName = this->baseItem->shortName;
		this->itemIndex = data.getProperty("index", m->this->items.indexOf(baseItem));

	}
}

template<class T>
BaseItem* Manager<T>::ItemBaseAction::getItem()
{
	if (baseItem != nullptr && !baseItem.wasObjectDeleted()) return baseItem.get();
	else
	{
		Manager* m = this->getManager();
		if (m != nullptr) return m->getItemWithName(itemShortName);
	}

	return nullptr;
}

template<class T>
Manager<T>::AddItemAction::AddItemAction(Manager* m, BaseItem* i, juce::var data) : ItemBaseAction(m, i, data) {
}

template<class T>
bool Manager<T>::AddItemAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr)
	{
		return false;
	}

	BaseItem* item = this->getItem();
	if (item != nullptr)
	{
		m->addItem(item, this->data, false);
	}
	else
	{
		item = m->addItemFromData(this->data, false);
	}

	if (item == nullptr) return false;

	this->itemShortName = item->shortName;
	return true;
}

template<class T>
bool Manager<T>::AddItemAction::undo()
{
	BaseItem* s = this->getItem();
	if (s == nullptr) return false;
	this->data = s->getJSONData();
	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(s, false);
	this->baseItem = nullptr;
	return true;
}

template<class T>
Manager<T>::RemoveItemAction::RemoveItemAction(Manager* m, BaseItem* i, juce::var data) : ItemBaseAction(m, i, data)
{

}

template<class T>
bool Manager<T>::RemoveItemAction::perform()
{

	if (this->baseItem == nullptr) return false;

	this->data = this->baseItem->getJSONData();
	if (this->data.getDynamicObject() == nullptr) return false;

	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(this->baseItem, false);
	this->baseItem = nullptr;
	return true;
}

template<class T>
bool Manager<T>::RemoveItemAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;
	this->baseItem = m->addItemFromData(this->data, false);
	return true;
}


template<class T>
Manager<T>::MoveItemAction::MoveItemAction(Manager* m, BaseItem* i, int prevIndex, int newIndex) :
	ItemBaseAction(m, i),
	prevIndex(prevIndex),
	newIndex(newIndex)
{

}

template<class T>
bool Manager<T>::MoveItemAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	BaseItem* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, newIndex, false);
	return true;
}

template<class T>
bool Manager<T>::MoveItemAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	BaseItem* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, prevIndex, false);
	return true;
}


template<class T>
Manager<T>::ItemsBaseAction::ItemsBaseAction(Manager* m, juce::Array<BaseItem*> iList, juce::var data) :
	ManagerBaseAction(m, data)
{
	if (data.isVoid())
	{
		std::sort(iList.begin(), iList.end(),
			[m](const auto& lhs, const auto& rhs) { return m->this->items.indexOf(lhs) < m->this->items.indexOf(rhs); });
	}

	for (auto& i : iList)
	{
		this->items.add(i);
		itemsShortName.add(i != nullptr ? i->shortName : "");
	}
}

template<class T>
juce::Array<BaseItem*> Manager<T>::ItemsBaseAction::getItems()
{
	juce::Array<BaseItem*> iList;
	int index = 0;
	for (auto& i : this->items)
	{
		if (i != nullptr && !i.wasObjectDeleted())
		{
			BaseItem* ti = dynamic_cast<BaseItem*>(i.get());
			if (ti != nullptr) iList.add(ti);
			else
			{
				Manager* m = this->getManager();
				if (m != nullptr)
				{
					ti = m->getItemWithName(this->itemsShortName[index]);
					if (ti != nullptr) iList.add(ti);
				}
			}
		}
		index++;
	}

	return iList;
}

template<class T>
Manager<T>::AddItemsAction::AddItemsAction(Manager* m, juce::Array<BaseItem*> iList, juce::var data) :
	ItemsBaseAction(m, iList, data)
{
}

template<class T>
bool Manager<T>::AddItemsAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	juce::Array<BaseItem*> iList = this->getItems();
	if (!iList.isEmpty()) m->addItems(iList, this->data, false);
	else if (!this->data.isVoid())
	{
		juce::Array<BaseItem*> newList = m->addItemsFromData(this->data, false);
		this->items.clear();
		for (auto& i : newList) this->items.add(i);
	}


	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}

template<class T>
bool Manager<T>::AddItemsAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr)
	{
		this->items.clear();
		return false;
	}

	juce::Array<BaseItem*> iList = this->getItems();
	this->data = juce::var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			juce::var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->this->items.indexOf(i));
			this->data.append(d);
		}
	}
	m->removeItems(iList, false);
	return true;
}

template<class T>
Manager<T>::RemoveItemsAction::RemoveItemsAction(Manager* m, juce::Array<BaseItem*> iList) : ItemsBaseAction(m, iList) {
}

template<class T>
bool Manager<T>::RemoveItemsAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr)
	{
		this->items.clear();
		return false;
	}

	juce::Array<BaseItem*> iList = this->getItems();
	this->data = juce::var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			juce::var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->this->items.indexOf(i));
			this->data.append(d);
		}
	}

	m->removeItems(iList, false);
	return true;
}

template<class T>
bool Manager<T>::RemoveItemsAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	juce::Array<BaseItem*> iList = m->addItemsFromData(this->data, false);

	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}



template<class T>
void Manager<T>::setHasGridOptions(bool hasGridOptions)
{
	if (hasGridOptions)
	{
		if (snapGridMode == nullptr)
		{
			snapGridMode = addBoolParameter("Snap Grid Mode", "If enabled, this will force moving objects snap to grid", false);
			showSnapGrid = addBoolParameter("Show Snap Grid", "If checked, this will show the snap grid", false);
			snapGridSize = addIntParameter("Snap Grid Size", "The size of the grid cells to snap to", 20, 4, 1000);
		}
	}
	else
	{
		if (snapGridMode != nullptr)
		{
			removeControllable(snapGridMode);
			removeControllable(showSnapGrid);
			removeControllable(snapGridSize);

			snapGridMode = nullptr;
			showSnapGrid = nullptr;
			snapGridSize = nullptr;
		}
	}
}

template<class T>
T* Manager<T>::createItem()
{
	if (managerFactory != nullptr && managerFactory->defs.size() == 1) return managerFactory->create(managerFactory->defs[0]);
	return new T();
}

template<class T>
BaseItem* Manager<T>::createItemFromData(juce::var data)
{
	if (canHaveGroups)
	{
		bool isGroup = data.getProperty("type", "").tojuce::String() == "ItemGroup";
		if (isGroup) return createGroup();
	}

	if (managerFactory != nullptr)
	{
		juce::String extendedType = data.getProperty("extendedType", "");
		if (extendedType != "") {
			BaseFactoryDefinition<T>* def = managerFactory->getDefFromExtendedType(extendedType);
			if (def == nullptr)
			{
				NLOGWARNING(niceName, "Could not find definition for extendedType \"" + extendedType + "\" in factory.");
				return nullptr;
			}
			return managerFactory->create(def);
		}
		juce::String type = data.getProperty("type", "");
		if (type.isEmpty()) return nullptr;
		return managerFactory->create(type);
	}


	return createItem();
}

template<class T>
void Manager<T>::notifyItemAdded(BaseItem* item, bool fromChildGroup)
{
	if (canHaveGroups)
	{
		if (Manager<T>* m = getParentAs<Manager<T>>(2)) m->notifyItemAdded(item, true);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	if (item->isGroup) listenersToCall->call(&ManagerListener::groupAdded, (G*)item);
	else listenersToCall->call(&ManagerListener::itemAdded, (T*)item);

	notifyAsync(ManagerEvent::ITEM_ADDED, (T*)item, fromChildGroup);
}

template<class T>
void Manager<T>::notifyItemsAdded(juce::Array<BaseItem*> addedItems, bool fromChildGroup)
{
	if (addedItems.isEmpty()) return;

	if (canHaveGroups)
	{
		if (Manager<T>* m = getParentAs<Manager<T>>(2)) m->notifyItemsAdded(addedItems, true);
	}

	juce::Array<T*> itemsToAdd;
	juce::Array<G*> groupsToAdd;
	for (auto& i : addedItems)
	{
		if (T* it = dynamic_cast<T*>(i)) itemsToAdd.add(it);
		else if (G* it2 = dynamic_cast<G*>(i)) groupsToAdd.add(it2);
	}
	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	if (!itemsToAdd.isEmpty()) listenersToCall->call(&ManagerListener::itemsAdded, itemsToAdd);
	if (!groupsToAdd.isEmpty()) listenersToCall->call(&ManagerListener::groupsAdded, groupsToAdd);

	notifyAsync(ManagerEvent::ITEMS_ADDED, addedItems, fromChildGroup);

}


template<class T>
void Manager<T>::notifyItemRemoved(BaseItem* removedItem, bool fromChildGroup)
{
	if (canHaveGroups)
	{
		if (Manager<T>* m = getParentAs<Manager<T>>(2)) m->notifyItemRemoved(removedItem, true);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	else listenersToCall->call(&ManagerListener::itemRemoved, removedItem);

	notifyAsync(ManagerEvent::ITEM_REMOVED, removedItem, fromChildGroup);
}

template<class T>
void Manager<T>::notifyItemsRemoved(juce::Array<BaseItem*> removedItems, bool fromChildGroup)
{
	if (removedItems.isEmpty()) return;

	if (canHaveGroups)
	{
		if (Manager<T>* m = getParentAs<Manager<T>>(2)) m->notifyItemsRemoved(removedItems, true);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	listenersToCall->call(&ManagerListener::itemsRemoved, removedItems);

	notifyAsync(ManagerEvent::ITEMS_REMOVED, removedItems, fromChildGroup);
}

template<class T>
void Manager<T>::notifyItemsReordered(bool fromChildGroup)
{
	if (canHaveGroups)
	{
		if (Manager<T>* m = getParentAs<Manager<T>>(2)) m->notifyItemsReordered(true);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	listenersToCall->call(&ManagerListener::itemsReordered);
	notifyAsync(ManagerEvent::ITEMS_REORDERED, juce::Array<BaseItem*>(), fromChildGroup);
}

template<class T>
int Manager<T>::getNumItems(bool recursive, bool includeDisabled, bool includeGroups) const
{
	return getAllItems(recursive, includeDisabled, includeGroups).size();
}

template<class T>
bool Manager<T>::hasNoItems(bool recursive, bool includeDisabled, bool includeGroups) const
{
	return getNumItems(recursive, includeDisabled, includeGroups) == 0;
}

template<class T>
juce::Array<BaseItem*> Manager<T>::getAllItems(bool recursive, bool includeDisabled, bool includeGroups) const
{
	juce::Array<BaseItem*> result;
	for (auto& i : this->items)
	{
		if (i->canBeDisabled && !i->enabled->boolValue() && !includeDisabled) continue;

		if (i->isGroup)
		{
			if (includeGroups) result.add((BaseItem*)i);
			if (recursive)
			{
				if (auto group = dynamic_cast<G*>(i))
					result.addArray(group->baseManager->getAllItems(recursive, includeDisabled, includeGroups));
			}
		}
		else
		{
			result.add(i);
		}
	}

	return result;
}

template<class T>
juce::Array<T*> Manager<T>::getItems(bool includeDisabled, bool recursive)
{
	return getAllItems(recursive, includeDisabled, false);
}

template<class T>
juce::Array<ItemGroup<T>*> Manager<T>::getGroups(bool includeDisabled, bool recursive)
{
	juce::Array<BaseItem*> _items = getAllItems(recursive, includeDisabled, true);
	juce::Array<G*> result;
	for (auto& i : this->items) if (i->isGroup) result.add(dynamic_cast<G*>(i));
	return result;
}

template<class T>
juce::PopupMenu Manager<T>::getItemsMenu(int startID)
{
	juce::PopupMenu menu;
	int numValues = items.size();
	for (int j = 0; j < numValues; j++)
	{
		T* c = items[j];
		menu.addItem(startID + j, c->niceName);
	}
	return menu;
}

template<class T>
T* Manager<T>::getItemForMenuResultID(int id, int startID)
{
	return items[id - startID];
}

template<class T>
T* Manager<T>::getFirstSelectedItem()
{
	for (auto& i : items) if (i->isSelected) return i;
	return nullptr;
}

template<class T>
void Manager<T>::notifyAsync(ManagerEventType type, juce::Array<BaseItem*> _items, bool fromChildGroup)
{
	ManagerNotifier* notifierToCall = fromChildGroup ? &recursiveManagerNotifier : &this->managerNotifier;
	notifierToCall->addMessage(new ManagerEvent(type, _items, fromChildGroup));
}


template<class T>
juce::Array<T*> Manager<T>::getAsItems(juce::Array<BaseItem*> _items) const
{
	juce::Array<T*> result;
	for (auto& i : _items) if (T* it = dynamic_cast<T*>(i)) result.add(it);
	return result;
}

template<class T>
juce::Array<ItemGroup<T>*> Manager<T>::getAsGroups(juce::Array<BaseItem*> _items) const
{
	juce::Array<ItemGroup<T>*> result;
	for (auto& i : _items) if (ItemGroup<T>* it = dynamic_cast<ItemGroup<T>*>(i)) result.add(it);
	return result;
}

template<class T>
T* Manager<T>::getAsItem(BaseItem* baseItem) const
{
	if (baseItem == nullptr) return nullptr;
	return dynamic_cast<T*>(baseItem);
}

template<class T>
ItemGroup<T>* Manager<T>::getAsGroup(BaseItem* baseItem) const
{
	if (baseItem == nullptr) return nullptr;
	return dynamic_cast<ItemGroup<T>*>(baseItem);
}

template<class T>
InspectableEditor* Manager<T>::getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables)
{
	return new GenericManagerEditor<T, G>(this, isRoot);
}


//SCRIPT
template<class T>
juce::var Manager<T>::addItemFromScript(const juce::var::NativeFunctionArgs& args)
{
	Manager<T>* m = getObjectFromJS<Manager<T>>(args);

	//if (args.numArguments) return juce::var(); 
	if (m->managerFactory == nullptr || m->managerFactory->defs.size() == 1)
	{
		T* item = m->addItem(nullptr, args.numArguments > 1 && args.arguments[1].isObject() ? args.arguments[1] : juce::var());
		if (item != nullptr) return item->getScriptObject();
	}
	else
	{
		if (args.numArguments < 1)
		{
			juce::String s = "Add item needs at least one parameter for this manager.\nValid options are :";
			for (auto& d : m->managerFactory->defs)
			{
				s += "\n" + d->type;
			}
			NLOGWARNING(m->niceName, "Error");// s);
			return juce::var();
		}
		else
		{
			T* item = m->managerFactory->create(args.arguments[0].tojuce::String());
			m->addItem(item, args.numArguments > 1 && args.arguments[1].isObject() ? args.arguments[1] : juce::var());
			if (item != nullptr) return item->getScriptObject();
		}
	}

	return juce::var();
}


template<class T>
juce::var Manager<T>::removeItemFromScript(const juce::var::NativeFunctionArgs& args)
{
	Manager<T>* m = getObjectFromJS<Manager<T>>(args);

	if (args.numArguments < 1)
	{
		NLOGWARNING(m->niceName, "Needs at least one argument to remove an item from this manager");
		return juce::var();
	}

	if (args.arguments[0].isObject())
	{
		T* item = dynamic_cast<T*>((T*)(juce::int64)(args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier)));
		if (item != nullptr)
		{
			m->removeItem(item);
			return juce::var();
		}
	}
	else if (args.arguments[0].isjuce::String())
	{
		T* item = m->getItemWithName(args.arguments[0].tojuce::String(), true);
		if (item != nullptr)
		{
			m->removeItem(item);
			return juce::var();
		}
	}

	NLOGWARNING(m->niceName, "Remove item : item not found in manager");
	return juce::var();
}

template<class T>
juce::var Manager<T>::removeAllItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		m->clear();
	}

	return juce::var();
}

template<class T>
juce::var Manager<T>::getItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	juce::var result = juce::var();
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		for (auto& i : m->items) result.append(i->getScriptObject());
	}

	return result;
}

template<class T>
juce::var Manager<T>::getItemWithNameFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		T* i = m->getItemWithName(args.arguments[0].tojuce::String(), true, true);
		if (i != nullptr) return i->getScriptObject();
	}

	return juce::var();
}

template<class T>
juce::var Manager<T>::getItemAtFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		int index = args.arguments[0];
		if (index < 0 || index >= m->items.size()) return juce::var();
		T* i = m->items[index];
		if (i != nullptr) return i->getScriptObject();
	}

	return juce::var();
}

template<class T>
juce::var Manager<T>::getItemIndexFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		if (!args.arguments[0].isObject()) return juce::var();
		T* item = dynamic_cast<T*>((T*)(juce::int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return juce::var();
		return m->items.indexOf(item);
	}

	return juce::var();
}

template<class T>
juce::var Manager<T>::getItemBeforeFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		if (!args.arguments[0].isObject()) return juce::var();
		T* item = dynamic_cast<T*>((T*)(juce::int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return juce::var();
		int index = m->items.indexOf(item);
		if (index <= 0) return juce::var();
		return m->items[index - 1]->getScriptObject();
	}

	return juce::var();
}

template<class T>
juce::var Manager<T>::getItemAfterFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		if (!args.arguments[0].isObject()) return juce::var();
		T* item = dynamic_cast<T*>((T*)(juce::int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return juce::var();
		int index = m->items.indexOf(item);
		if (index >= m->items.size() - 1) return juce::var();
		return m->items[index + 1]->getScriptObject();
	}

	return juce::var();
}

template<class T>
juce::var Manager<T>::reorderItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T>* m = getObjectFromJS<Manager<T>>(args))
	{
		m->reorderItems();
	}

	return juce::var();
}


//MANAGER EVENT
template<class T>
Manager<T>::ManagerItemComparator::ManagerItemComparator(Manager* manager) : m(manager), compareFunc(nullptr)
{
	compareFunc = nullptr;
}

template<class T>
int Manager<T>::ManagerItemComparator::compareElements(ControllableContainer* i1, ControllableContainer* i2)
{
	jassert(compareFunc != nullptr);
	return compareFunc(static_cast<T*>(i1), static_cast<T*>(i2));
}

template<class T>
template<class IType>
juce::Array<IType*> Manager<T>::getItemsWithType(bool recursive)
{
	juce::Array<BaseItem*> itemsToSearch = getAllitems(recursive, true, false);
	juce::Array<IType*> result;
	for (auto& i : itemsToSearch)
	{
		if (IType* it = dynamic_cast<IType*>(i)) result.add(it);
	}

	return result;
}