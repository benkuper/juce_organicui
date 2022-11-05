/*
  ==============================================================================

	BaseManager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once


#define DECLARE_TYPE(type) String getTypeString() const override { return getTypeStringStatic() ; } \
static String getTypeStringStatic() { return type; }


template <class T>
class BaseManager :
	public EnablingControllableContainer,
	public BaseItemListener
{
public:
	BaseManager<T>(const String& name);
	virtual ~BaseManager<T>();

	OwnedArray<T, CriticalSection> items;
	bool isClearing;

	//Factory
	Factory<T>* managerFactory;
	String itemDataType;

	bool userCanAddItemsManually;
	bool selectItemWhenCreated;
	bool autoReorderOnAdd;
	bool isManipulatingMultipleItems;
	Point<float> clipboardCopyOffset;

	//ui
	Point<int> viewOffset; //in pixels, viewOffset of 0 means zeroPos is at the center of the window
	//interaction
	float viewZoom;

	//grid
	BoolParameter* snapGridMode;
	BoolParameter* showSnapGrid;
	IntParameter* snapGridSize;

	std::function<T* ()> customCreateItemFunc;

	void setHasGridOptions(bool hasGridOptions);

	virtual T* createItem(); //to override if special constructor to use
	virtual T* createItemFromData(var data); //to be overriden for specific item creation (from data)
	virtual T* addItemFromData(var data, bool addToUndo = true); //to be overriden for specific item creation (from data)
	virtual Array<T*> addItemsFromData(var data, bool addToUndo = true); //to be overriden for specific item creation (from data)
	virtual Array<T*> addItemsFromClipboard(bool showWarning = true);
	virtual bool canAddItemOfType(const String& typeToCheck);

	virtual void loadItemsData(var data);


	virtual UndoableAction* getAddItemUndoableAction(T* item = nullptr, var data = var());
	virtual UndoableAction* getAddItemsUndoableAction(Array<T*> item = nullptr, var data = var());

	T* addItem(T* item = nullptr, var data = var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data
	T* addItem(const Point<float> initialPosition, bool addToUndo = true, bool notify = true);
	T* addItem(T* item, const Point<float> initialPosition, bool addToUndo = true, bool notify = true);
	Array<T*> addItems(Array<T*> items, var data = var(), bool addToUndo = true);


	virtual Array<UndoableAction*> getRemoveItemUndoableAction(T* item);
	virtual Array<UndoableAction*> getRemoveItemsUndoableAction(Array<T*> items);

	void removeItems(Array<T*> items, bool addToUndo = true);
	T* removeItem(T* item, bool addToUndo = true, bool notify = true, bool returnItem = false);

	virtual void setItemIndex(T* item, int newIndex, bool addToUndo = true);
	virtual Array<UndoableAction*> getSetItemIndexUndoableAction(T* item, int newIndex);

	virtual void reorderItems(); //to be overriden if needed


	//to override for specific handling like adding custom listeners, etc.
	virtual void addItemInternal(T*, var data) {}
	virtual void addItemsInternal(Array<T*>, var data) {}
	virtual void removeItemInternal(T*) {}
	virtual void removeItemsInternal(Array<T*>) {}


	T* getItemWithName(const String& itemShortName, bool searchNiceNameToo = false, bool searchWithLowerCaseIfNotFound = true);

	template<class IType>
	Array<IType*> getItemsWithType();

	virtual void clear() override;
	void askForRemoveBaseItem(BaseItem* item) override;
	void askForDuplicateItem(BaseItem* item) override;
	void askForPaste() override;
	void askForMoveBefore(BaseItem*) override;
	void askForMoveAfter(BaseItem*) override;
	void askForSelectAllItems(bool addToSelection = false) override;
	void askForSelectPreviousItem(BaseItem* item, bool addToSelection = false) override;
	void askForSelectNextItem(BaseItem* item, bool addToSelection = false) override;

	void onContainerParameterChanged(Parameter* p) override;

	virtual var getExportSelectionData();
	virtual var getJSONData() override;
	virtual void loadJSONDataInternal(var data) override;
	virtual void loadJSONDataManagerInternal(var data);

	PopupMenu getItemsMenu(int startID);
	T* getItemForMenuResultID(int id, int startID);

	T* getFirstSelectedItem();

	String getScriptTargetString() override;

	//using BManagerListener = typename BaseManagerListener<T>;
	//friend class BaseManagerListener<T>;

	typedef BaseManagerListener<T> ManagerListener;

	ListenerList<ManagerListener> baseManagerListeners;
	void addBaseManagerListener(ManagerListener* newListener) { baseManagerListeners.add(newListener); }
	void removeBaseManagerListener(ManagerListener* listener) { baseManagerListeners.remove(listener); }

	class ManagerEvent
	{
	public:
		enum Type { ITEM_ADDED, ITEM_REMOVED, ITEMS_REORDERED, ITEMS_ADDED, ITEMS_REMOVED, MANAGER_CLEARED, NEEDS_UI_UPDATE };

		ManagerEvent(Type t, T* i = nullptr);
		ManagerEvent(Type t, Array<T*> iList);

		Type type;
		Array<WeakReference<Inspectable>> itemsRef;
		Array<T*> getItems() const;
		T* getItem(int index = 0) const;
	};

	using BManagerEvent = typename BaseManager<T>::ManagerEvent;
	using ManagerNotifier = QueuedNotifier<BManagerEvent>;
	ManagerNotifier managerNotifier;
	typedef typename QueuedNotifier<BManagerEvent>::Listener AsyncListener;

	void addAsyncManagerListener(AsyncListener* newListener) { managerNotifier.addListener(newListener); }
	void addAsyncCoalescedManagerListener(AsyncListener* newListener) { managerNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncManagerListener(AsyncListener* listener) { managerNotifier.removeListener(listener); }


	InspectableEditor* getEditorInternal(bool isRoot, Array<Inspectable*> inspectables = Array<Inspectable*>()) override;

	//UNDO MANAGER
	class ManagerBaseAction :
		public UndoableAction
	{
	public:
		ManagerBaseAction(BaseManager* manager, var _data = var());

		String managerControlAddress;
		var data;
		WeakReference<Inspectable> managerRef;

		BaseManager<T>* getManager();
	};

	class ItemBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemBaseAction(BaseManager* m, T* i, var data = var());

		WeakReference<Inspectable> itemRef;
		String itemShortName;
		int itemIndex;

		T* getItem();
	};

	class AddItemAction :
		public ItemBaseAction
	{
	public:
		AddItemAction(BaseManager* m, T* i, var data = var());

		bool perform() override;
		bool undo() override;
	};

	class RemoveItemAction :
		public ItemBaseAction
	{
	public:
		RemoveItemAction(BaseManager* m, T* i, var data = var());

		bool perform() override;
		bool undo() override;
	};

	class MoveItemAction :
		public ItemBaseAction
	{
	public:
		MoveItemAction(BaseManager* m, T* i, int prevIndex, int newIndex);

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
		ItemsBaseAction(BaseManager* m, Array<T*> iList, var data = var());

		Array<WeakReference<Inspectable>> itemsRef;
		StringArray itemsShortName;

		Array<T*> getItems();
	};

	class AddItemsAction :
		public ItemsBaseAction
	{
	public:
		AddItemsAction(BaseManager* m, Array<T*> iList, var data = var());

		int startIndex;
		bool perform() override;
		bool undo() override;
	};


	class RemoveItemsAction :
		public ItemsBaseAction
	{
	public:
		RemoveItemsAction(BaseManager* m, Array<T*> iList);

		bool perform() override;
		bool undo() override;
	};


	class ManagerItemComparator
	{
	public:
		ManagerItemComparator(BaseManager* manager);

		BaseManager* m;

		std::function<int(T*, T*)> compareFunc;
		int compareElements(ControllableContainer* i1, ControllableContainer* i2);

	};

	ManagerItemComparator comparator;


	static var addItemFromScript(const var::NativeFunctionArgs& args);
	static var removeItemFromScript(const var::NativeFunctionArgs& args);
	static var removeAllItemsFromScript(const var::NativeFunctionArgs& args);
	static var getItemsFromScript(const var::NativeFunctionArgs& args);
	static var getItemWithNameFromScript(const var::NativeFunctionArgs& args);
	static var getItemAtFromScript(const var::NativeFunctionArgs& args);
	static var getItemIndexFromScript(const var::NativeFunctionArgs& args);
	static var getItemBeforeFromScript(const var::NativeFunctionArgs& args);
	static var getItemAfterFromScript(const var::NativeFunctionArgs& args);
	static var reorderItemsFromScript(const var::NativeFunctionArgs& args);

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseManager<T>)
};


template<class T>
BaseManager<T>::BaseManager(const String& name) :
	EnablingControllableContainer(name, false),
	isClearing(false),
	managerFactory(nullptr),
	itemDataType(""),
	userCanAddItemsManually(true),
	selectItemWhenCreated(true),
	autoReorderOnAdd(true),
	isManipulatingMultipleItems(false),
	viewZoom(1),
	snapGridMode(nullptr),
	showSnapGrid(nullptr),
	snapGridSize(nullptr),
	customCreateItemFunc(nullptr),
	managerNotifier(50),
	comparator(this)
{

	scriptObject.setMethod("addItem", &BaseManager<T>::addItemFromScript);
	scriptObject.setMethod("removeItem", &BaseManager<T>::removeItemFromScript);
	scriptObject.setMethod("removeAll", &BaseManager<T>::removeAllItemsFromScript);
	scriptObject.setMethod("getItems", &BaseManager<T>::getItemsFromScript);
	scriptObject.setMethod("getItemWithName", &BaseManager<T>::getItemWithNameFromScript);
	scriptObject.setMethod("getItemAt", &BaseManager<T>::getItemAtFromScript);
	scriptObject.setMethod("getItemIndex", &BaseManager<T>::getItemIndexFromScript);
	scriptObject.setMethod("getItemBefore", &BaseManager<T>::getItemBeforeFromScript);
	scriptObject.setMethod("getItemAfter", &BaseManager<T>::getItemAfterFromScript);
	scriptObject.setMethod("reorderItems", &BaseManager<T>::reorderItemsFromScript);

	skipLabelInTarget = true; //by default manager label in targetParameter UI are not interesting
	nameCanBeChangedByUser = false;


}

template<class T>
BaseManager<T>::~BaseManager()
{

	clear();
}

template<class T>
template<class IType>
Array<IType*> BaseManager<T>::getItemsWithType()
{
	Array<IType*> result;
	for (auto& i : items)
	{
		if (IType* it = dynamic_cast<IType*>(i)) result.add(it);
	}

	return result;
}

template<class T>
void BaseManager<T>::setHasGridOptions(bool hasGridOptions)
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
T* BaseManager<T>::createItem()
{
	if (managerFactory != nullptr && managerFactory->defs.size() == 1) return managerFactory->create(managerFactory->defs[0]);
	if (customCreateItemFunc != nullptr) return customCreateItemFunc();
	return new T();
}

template<class T>
T* BaseManager<T>::createItemFromData(var data)
{
	if (managerFactory != nullptr)
	{
		String type = data.getProperty("type", "");
		if (type.isEmpty()) return nullptr;
		return managerFactory->create(type);
	}

	return createItem();
}

template<class T>
UndoableAction* BaseManager<T>::getAddItemUndoableAction(T* item, var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	jassert(items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	return new AddItemAction(this, item, data);
}

template<class T>
UndoableAction* BaseManager<T>::getAddItemsUndoableAction(Array<T*> _items, var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	if (_items.size() == 0) return nullptr;
	return new AddItemsAction(this, _items, data);
}

template<class T>
T* BaseManager<T>::addItem(T* item, var data, bool addToUndo, bool notify)
{

	jassert(items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	BaseItem* bi = static_cast<BaseItem*>(item);

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performAction("Add " + bi->niceName, new AddItemAction(this, item, data));
			return item;
		}
	}

	int targetIndex = data.getProperty("index", -1);
	if (targetIndex != -1) items.insert(targetIndex, item);
	else
	{
		//items.getLock().enter();
		if (autoReorderOnAdd && !isCurrentlyLoadingData && comparator.compareFunc != nullptr) items.addSorted(comparator, item);
		else items.add(item);
		//items.getLock().exit();
	}

	bi->addBaseItemListener(this);

	if (!data.isVoid())
	{
		bi->loadJSONData(data);
	}

	//bi->setNiceName(bi->niceName); //force setting a unique name if already taken, after load data so if name is the same as another, will change here

	addChildControllableContainer(bi, false, items.indexOf(item), notify);

	//if(autoReorderOnAdd) reorderItems();

	addItemInternal(item, data);

	if (notify)
	{
		baseManagerListeners.call(&BaseManagerListener<T>::itemAdded, item);
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_ADDED, item));

	}

	if (selectItemWhenCreated && !isCurrentlyLoadingData && !isManipulatingMultipleItems) bi->selectThis();

	return item;
}

template<class T>
T* BaseManager<T>::addItem(const Point<float> initialPosition, bool addToUndo, bool notify)
{
	return addItem(nullptr, initialPosition, addToUndo, notify);
}

template<class T>
T* BaseManager<T>::addItem(T* item, const Point<float> initialPosition, bool addToUndo, bool notify)
{
	if (item == nullptr) item = createItem();
	item->viewUIPosition->setPoint(initialPosition);
	addItem(item, addToUndo, notify);
	return item;
}

template<class T>
Array<T*> BaseManager<T>::addItems(Array<T*> itemsToAdd, var data, bool addToUndo)
{

	isCurrentlyLoadingData = true;
	isManipulatingMultipleItems = true;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		AddItemsAction* a = new AddItemsAction(this, itemsToAdd, data);
		UndoMaster::getInstance()->performAction("Add " + String(itemsToAdd.size()) + " items", a);
		isCurrentlyLoadingData = false;
		isManipulatingMultipleItems = false;
		return itemsToAdd;
	}


	for (int i = 0; i < itemsToAdd.size(); ++i)
	{
		addItem(itemsToAdd[i], data.isArray() ? data[i] : var(), false, false);
	}

	notifyStructureChanged();

	baseManagerListeners.call(&BaseManagerListener<T>::itemsAdded, itemsToAdd);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_ADDED, itemsToAdd));

	reorderItems();
	isCurrentlyLoadingData = false;
	isManipulatingMultipleItems = false;

	addItemsInternal(itemsToAdd, data);

	return itemsToAdd;
}

//if data is not empty, load data
template<class T>
T* BaseManager<T>::addItemFromData(var data, bool addToUndo)
{
	T* item = createItemFromData(data);
	if (item == nullptr) return nullptr;
	return addItem(item, data, addToUndo);
}

template<class T>
Array<T*> BaseManager<T>::addItemsFromData(var data, bool addToUndo)
{
	Array<T*> itemsToAdd;
	if (managerFactory != nullptr)
	{
		for (int i = 0; i < data.size(); ++i)
		{
			String type = data[i].getProperty("type", "");
			if (type.isEmpty()) return nullptr;
			T* it = managerFactory->create(type);
			if (it != nullptr) itemsToAdd.add(it);
		}
	}
	else
	{
		for (int i = 0; i < data.size(); ++i)  itemsToAdd.add(createItem());
	}

	return addItems(itemsToAdd, data, addToUndo);
}

template<class T>
Array<T*> BaseManager<T>::addItemsFromClipboard(bool showWarning)
{
	if (!userCanAddItemsManually) return Array<T*>();
	String s = SystemClipboard::getTextFromClipboard();
	var data = JSON::parse(s);
	if (data.isVoid()) return Array<T*>();

	String t = data.getProperty("itemType", "");
	if (!canAddItemOfType(t))
	{
		if (showWarning) NLOGWARNING(niceName, "Can't paste data from clipboard : data is of wrong type (\"" + t + "\").");
		return Array<T*>();
	}

	var itemsData = data.getProperty("items", var());
	int sIndex = items.indexOf(InspectableSelectionManager::activeSelectionManager->getInspectableAs<T>());
	if (sIndex >= 0)
	{
		sIndex++;
		for (int i = 0; i < itemsData.size(); i++)
		{
			itemsData[i].getDynamicObject()->setProperty("index", sIndex++);
		}
	}

	Array<T*> copiedItems = addItemsFromData(itemsData);

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
bool BaseManager<T>::canAddItemOfType(const String& typeToCheck)
{
	return typeToCheck == itemDataType;
}

template<class T>
void BaseManager<T>::loadItemsData(var data)
{
	if (data == var()) return;
	Array<var>* itemsData = data.getProperty("items", var()).getArray();
	if (itemsData == nullptr) return;

	for (auto& td : *itemsData)
	{
		String n = td.getProperty("niceName", "");
		BaseItem* i = getItemWithName(n, true);
		if (i != nullptr) i->loadJSONData(td);
	}
}

template<class T>
Array<UndoableAction*> BaseManager<T>::getRemoveItemUndoableAction(T* item)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	Array<UndoableAction*> a;
	a.add(new RemoveItemAction(this, item));
	return a;
}

template<class T>
Array<UndoableAction*> BaseManager<T>::getRemoveItemsUndoableAction(Array<T*> itemsToRemove)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	Array<UndoableAction*> a;
	a.add(new RemoveItemsAction(this, itemsToRemove));
	return a;
}

template<class T>
void BaseManager<T>::removeItems(Array<T*> itemsToRemove, bool addToUndo)
{
	isManipulatingMultipleItems = true;
	if (addToUndo)
	{
		Array<UndoableAction*> a = getRemoveItemsUndoableAction(itemsToRemove);
		UndoMaster::getInstance()->performActions("Remove " + String(itemsToRemove.size()) + " items", a);
		isManipulatingMultipleItems = false;
		return;
	}

	Array<T*> itemsRemoved;
	for (auto& i : itemsToRemove) itemsRemoved.add(removeItem(i, false, false, true));

	removeItemsInternal(itemsRemoved);

	baseManagerListeners.call(&BaseManagerListener<T>::itemsRemoved, itemsRemoved);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REMOVED));

	for (auto& i : itemsRemoved)
	{
		((BaseItem*)i)->clearItem();
		delete i;
	}

	isManipulatingMultipleItems = false;
}

template<class T>
T* BaseManager<T>::removeItem(T* item, bool addToUndo, bool notify, bool returnItem)
{
	if (item == nullptr) return nullptr;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			BaseItem* bi = static_cast<BaseItem*>(item);
			UndoMaster::getInstance()->performActions("Remove " + bi->getTypeString(), getRemoveItemUndoableAction(item));
			return nullptr;
		}
	}


	//items.getLock().enter();
	items.removeObject(item, false);
	//items.getLock().exit();

	removeItemInternal(item);

	BaseItem* bi = static_cast<BaseItem*>(item);
	bi->removeBaseItemListener(this);
	removeChildControllableContainer(bi);

	if (notify)
	{
		baseManagerListeners.call(&BaseManagerListener<T>::itemRemoved, item);
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_REMOVED, item));
	}

	if (returnItem) return item; //will need to delete !

	bi->clearItem();
	delete item;

	return nullptr;
}

template<class T>
void BaseManager<T>::setItemIndex(T* item, int newIndex, bool addToUndo)
{
	if (item == nullptr) return;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			BaseItem* bi = static_cast<BaseItem*>(item);
			UndoMaster::getInstance()->performActions("Move " + bi->getTypeString(), getSetItemIndexUndoableAction(item, newIndex));
			return;
		}
	}


	newIndex = jlimit(0, items.size() - 1, newIndex);
	int index = items.indexOf(item);
	if (index == -1 || index == newIndex) return;

	//items.getLock().enter();
	items.move(index, newIndex);
	controllableContainers.move(index, newIndex);
	//items.getLock().exit();

	baseManagerListeners.call(&BaseManagerListener<T>::itemsReordered);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T>
Array<UndoableAction*> BaseManager<T>::getSetItemIndexUndoableAction(T* item, int newIndex)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	Array<UndoableAction*> a;
	a.add(new MoveItemAction(this, item, this->items.indexOf(item), newIndex));
	return a;
}

template<class T>
void BaseManager<T>::reorderItems()
{
	if (comparator.compareFunc != nullptr)
	{
		//items.getLock().enter();
		items.sort(comparator);
		//items.getLock().exit();
		controllableContainers.clear();
		controllableContainers.addArray(items);
	}

	baseManagerListeners.call(&BaseManagerListener<T>::itemsReordered);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T>
T* BaseManager<T>::getItemWithName(const String& itemShortName, bool searchItemWithNiceNameToo, bool searchWithLowerCaseIfNotFound)
{
	//const ScopedLock lock(items.getLock());
	for (auto& t : items)
	{
		if (((BaseItem*)t)->shortName == itemShortName) return t;
		else if (searchItemWithNiceNameToo && ((BaseItem*)t)->niceName == itemShortName) return t;
	}

	if (searchWithLowerCaseIfNotFound)
	{
		for (auto& t : items)
		{
			if (((BaseItem*)t)->shortName.toLowerCase() == itemShortName.toLowerCase()) return t;
		}
	}

	return nullptr;
}

template<class T>
void BaseManager<T>::clear()
{
	isClearing = true;
	//const ScopedLock lock(items.getLock());
	while (items.size() > 0) removeItem(items[0], false);
}

template<class T>
void BaseManager<T>::askForRemoveBaseItem(BaseItem* item)
{
	removeItem(static_cast<T*>(item));
}

template<class T>
void BaseManager<T>::askForDuplicateItem(BaseItem* item)
{
	if (!userCanAddItemsManually) return;
	var data = item->getJSONData();
	data.getDynamicObject()->setProperty("index", items.indexOf(static_cast<T*>(item)) + 1);
	addItemFromData(data);
}

template<class T>
void BaseManager<T>::askForPaste()
{
	addItemsFromClipboard();
}

template<class T>
void BaseManager<T>::askForMoveBefore(BaseItem* i)
{
	T* item = static_cast<T*>(i);
	setItemIndex(item, jmax(items.indexOf(item) - 1, 0));
	//	int index = items.indexOf(static_cast<T *>(i));
	//	if (index == 0) return;
	//	items.swap(index, index - 1);
	//	controllableContainers.swap(index, index - 1);
	//
	//	baseManagerListeners.call(&ManagerListener::itemsReordered);
	//	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T>
void BaseManager<T>::askForMoveAfter(BaseItem* i)
{
	T* item = static_cast<T*>(i);
	setItemIndex(item, jmin(items.indexOf(item) + 1, items.size() - 1));
	//int index = items.indexOf(static_cast<T *>(i));
	//if (index == items.size() -1) return;
	//items.swap(index, index + 1);
	//controllableContainers.swap(index, index+1);

	//baseManagerListeners.call(&ManagerListener::itemsReordered);
	//managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T>
void BaseManager<T>::askForSelectAllItems(bool addToSelection)
{
	int numItems = items.size();
	if (!addToSelection) selectionManager->clearSelection(numItems == 0);
	else deselectThis(numItems == 0);

	if (numItems > 1) for (int i = 0; i < numItems; ++i) items[i]->selectThis(true, i == numItems - 1); //only notify on last
	else if (numItems > 0) items[0]->selectThis(addToSelection, true);
}

template<class T>
void BaseManager<T>::askForSelectPreviousItem(BaseItem* item, bool addToSelection)
{
	int index = items.indexOf(dynamic_cast<T*>(item));
	if (index <= 0) return;
	items[index - 1]->selectThis(addToSelection);
}

template<class T>
void BaseManager<T>::askForSelectNextItem(BaseItem* item, bool addToSelection)
{
	int index = items.indexOf(dynamic_cast<T*>(item));
	if (index == -1 || index >= items.size() - 1) return;
	items[index + 1]->selectThis(addToSelection);
}

template<class T>
void BaseManager<T>::onContainerParameterChanged(Parameter* p)
{
	EnablingControllableContainer::onContainerParameterChanged(p);
	if (p == showSnapGrid || p == snapGridSize)
	{
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::NEEDS_UI_UPDATE));
	}
}

template<class T>
var BaseManager<T>::getExportSelectionData()
{
	var data;

	for (auto& t : items)
	{
		if (t->isSavable && t->isSelected) data.append(t->getJSONData());
	}

	return data;
}

template<class T>
var BaseManager<T>::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	var itemsData = var();
	//items.getLock().enter();
	for (auto& t : items)
	{
		if (t->isSavable) itemsData.append(t->getJSONData());
	}
	//items.getLock().exit();

	if (itemsData.size() > 0) data.getDynamicObject()->setProperty("items", itemsData);

	var vData;
	vData.append(viewOffset.x);
	vData.append(viewOffset.y);
	data.getDynamicObject()->setProperty("viewOffset", vData);
	data.getDynamicObject()->setProperty("viewZoom", viewZoom);

	return data;
}

template<class T>
void BaseManager<T>::loadJSONDataInternal(var data)
{
	clear();

	if (data.hasProperty("viewOffset"))
	{
		var vData = data.getProperty("viewOffset", var());
		viewOffset.setXY(vData[0], vData[1]);
	}

	viewZoom = data.getProperty("viewZoom", 1);

	loadJSONDataManagerInternal(data);
}

template<class T>
void BaseManager<T>::loadJSONDataManagerInternal(var data)
{
	Array<var>* itemsData = data.getProperty("items", var()).getArray();
	if (itemsData == nullptr) return;

	for (auto& td : *itemsData)
	{
		addItemFromData(td, false);
	}
}

template<class T>
PopupMenu BaseManager<T>::getItemsMenu(int startID)
{
	PopupMenu menu;
	int numValues = items.size();
	for (int j = 0; j < numValues; j++)
	{
		T* c = items[j];
		menu.addItem(startID + j, c->niceName);
	}
	return menu;
}

template<class T>
T* BaseManager<T>::getItemForMenuResultID(int id, int startID)
{
	return items[id - startID];
}

template<class T>
T* BaseManager<T>::getFirstSelectedItem()
{
	for (auto& i : items) if (i->isSelected) return i;
	return nullptr;
}

template<class T>
String BaseManager<T>::getScriptTargetString()
{
	return "[" + niceName + " : Manager(" + itemDataType + ")]";
}


template<class T>
InspectableEditor* BaseManager<T>::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new GenericManagerEditor<T>(this, isRoot);

}


// SCRIPT

template<class T>
var BaseManager<T>::addItemFromScript(const var::NativeFunctionArgs& args)
{
	BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args);

	//if (args.numArguments) return var(); 
	if (m->managerFactory == nullptr || m->managerFactory->defs.size() == 1)
	{
		T* item = m->addItem(nullptr, args.numArguments > 1 && args.arguments[1].isObject() ? args.arguments[1] : var());
		return item->getScriptObject();
	}
	else
	{
		if (args.numArguments < 1)
		{
			String s = "Add item needs at least one parameter for this manager.\nValid options are :";
			for (auto& d : m->managerFactory->defs)
			{
				s += "\n" + d->type;
			}
			NLOGWARNING(m->niceName, "Error");// s);
			return var();
		}
		else
		{
			T* item = m->managerFactory->create(args.arguments[0].toString());
			m->addItem(item, args.numArguments > 1 && args.arguments[1].isObject() ? args.arguments[1] : var());
			if (item != nullptr) return item->getScriptObject();
		}

		return var();
	}
}


template<class T>
var BaseManager<T>::removeItemFromScript(const var::NativeFunctionArgs& args)
{
	BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args);

	if (args.numArguments < 1)
	{
		NLOGWARNING(m->niceName, "Needs at least one argument to remove an item from this manager");
		return var();
	}

	if (args.arguments[0].isObject())
	{
		T* item = dynamic_cast<T*>((T*)(int64)(args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier)));
		if (item != nullptr)
		{
			m->removeItem(item);
			return var();
		}
	}
	else if (args.arguments[0].isString())
	{
		T* item = m->getItemWithName(args.arguments[0].toString(), true);
		if (item != nullptr)
		{
			m->removeItem(item);
			return var();
		}
	}

	NLOGWARNING(m->niceName, "Remove item : item not found in manager");
	return var();
}

template<class T>
var BaseManager<T>::removeAllItemsFromScript(const var::NativeFunctionArgs& args)
{
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		m->clear();
	}

	return var();
}

template<class T>
var BaseManager<T>::getItemsFromScript(const var::NativeFunctionArgs& args)
{
	var result = var();
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		for (auto& i : m->items) result.append(i->getScriptObject());
	}

	return result;
}

template<class T>
var BaseManager<T>::getItemWithNameFromScript(const var::NativeFunctionArgs& args)
{
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return var();
		T* i = m->getItemWithName(args.arguments[0].toString(), true, true);
		if (i != nullptr) return i->getScriptObject();
	}

	return var();
}

template<class T>
var BaseManager<T>::getItemAtFromScript(const var::NativeFunctionArgs& args)
{
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return var();
		int index = args.arguments[0];
		if (index < 0 || index >= m->items.size()) return var();
		T* i = m->items[index];
		if (i != nullptr) return i->getScriptObject();
	}

	return var();
}

template<class T>
var BaseManager<T>::getItemIndexFromScript(const var::NativeFunctionArgs& args)
{
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return var();
		if (!args.arguments[0].isObject()) return var();
		T* item = dynamic_cast<T*>((T*)(int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return var();
		return m->items.indexOf(item);
	}

	return var();
}

template<class T>
var BaseManager<T>::getItemBeforeFromScript(const var::NativeFunctionArgs& args)
{
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return var();
		if (!args.arguments[0].isObject()) return var();
		T* item = dynamic_cast<T*>((T*)(int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return var();
		int index = m->items.indexOf(item);
		if (index <= 0) return var();
		return m->items[index - 1]->getScriptObject();
	}

	return var();
}

template<class T>
var BaseManager<T>::getItemAfterFromScript(const var::NativeFunctionArgs& args)
{
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return var();
		if (!args.arguments[0].isObject()) return var();
		T* item = dynamic_cast<T*>((T*)(int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return var();
		int index = m->items.indexOf(item);
		if (index >= m->items.size() - 1) return var();
		return m->items[index + 1]->getScriptObject();
	}

	return var();
}

template<class T>
var BaseManager<T>::reorderItemsFromScript(const var::NativeFunctionArgs& args)
{
	if (BaseManager<T>* m = getObjectFromJS<BaseManager<T>>(args))
	{
		m->reorderItems();
	}

	return var();
}


//MANAGER EVENT

template<class T>
BaseManager<T>::ManagerEvent::ManagerEvent(Type t, T* i) : type(t)
{
	itemsRef.add(i);
}

template<class T>
BaseManager<T>::ManagerEvent::ManagerEvent(Type t, Array<T*> iList) : type(t)
{
	for (auto& i : iList)
	{
		itemsRef.add(dynamic_cast<Inspectable*>(i));
	}
}

template<class T>
Array<T*> BaseManager<T>::ManagerEvent::getItems() const
{
	Array<T*> result;
	for (auto& i : itemsRef)
	{
		if (i != nullptr && !i.wasObjectDeleted()) result.add(dynamic_cast<T*>(i.get()));
	}
	return result;
}

template<class T>
T* BaseManager<T>::ManagerEvent::getItem(int index) const
{
	if (itemsRef.size() > index && itemsRef[index] != nullptr && !itemsRef[index].wasObjectDeleted()) return static_cast<T*>(itemsRef[index].get());
	return nullptr;
}







//ACTIONS

template<class T>
BaseManager<T>::ManagerBaseAction::ManagerBaseAction(BaseManager* manager, var _data) :
	managerControlAddress(manager->getControlAddress()),
	data(_data),
	managerRef(manager)
{}

template<class T>
BaseManager<T>* BaseManager<T>::ManagerBaseAction::getManager() {
	if (managerRef != nullptr && !managerRef.wasObjectDeleted()) return dynamic_cast<BaseManager<T>*>(managerRef.get());
	else if (Engine::mainEngine != nullptr)
	{
		ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(managerControlAddress, true);
		if (cc != nullptr) return dynamic_cast<BaseManager<T>*>(cc);
	}

	return nullptr;
}

template<class T>
BaseManager<T>::ItemBaseAction::ItemBaseAction(BaseManager* m, T* i, var data) :
	ManagerBaseAction(m, data),
	itemRef(i),
	itemIndex(0)
{
	T* s = getItem();
	if (s != nullptr)
	{
		this->itemShortName = dynamic_cast<BaseItem*>(s)->shortName;
		this->itemIndex = data.getProperty("index", m->items.indexOf(s));

	}
}

template<class T>
T* BaseManager<T>::ItemBaseAction::getItem()
{
	if (itemRef != nullptr && !itemRef.wasObjectDeleted()) return dynamic_cast<T*>(itemRef.get());
	else
	{
		BaseManager* m = this->getManager();
		if (m != nullptr) return m->getItemWithName(itemShortName);
	}

	return nullptr;
}

template<class T>
BaseManager<T>::AddItemAction::AddItemAction(BaseManager* m, T* i, var data) : ItemBaseAction(m, i, data) {
}

template<class T>
bool BaseManager<T>::AddItemAction::perform()
{
	BaseManager* m = this->getManager();
	if (m == nullptr)
	{
		return false;
	}

	T* item = this->getItem();
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
bool BaseManager<T>::AddItemAction::undo()
{
	T* s = this->getItem();
	if (s == nullptr) return false;
	this->data = s->getJSONData();
	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(s, false);
	this->itemRef = nullptr;
	return true;
}

template<class T>
BaseManager<T>::RemoveItemAction::RemoveItemAction(BaseManager* m, T* i, var data) : ItemBaseAction(m, i, data)
{

}

template<class T>
bool BaseManager<T>::RemoveItemAction::perform()
{

	T* s = this->getItem();

	if (s == nullptr) return false;

	this->data = s->getJSONData();
	if (this->data.getDynamicObject() == nullptr) return false;

	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(s, false);
	this->itemRef = nullptr;
	return true;
}

template<class T>
bool BaseManager<T>::RemoveItemAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;
	this->itemRef = m->addItemFromData(this->data, false);
	return true;
}


template<class T>
BaseManager<T>::MoveItemAction::MoveItemAction(BaseManager* m, T* i, int prevIndex, int newIndex) :
	ItemBaseAction(m, i),
	prevIndex(prevIndex),
	newIndex(newIndex)
{

}

template<class T>
bool BaseManager<T>::MoveItemAction::perform()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	T* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, newIndex, false);
	return true;
}

template<class T>
bool BaseManager<T>::MoveItemAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	T* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, prevIndex, false);
	return true;
}


template<class T>
BaseManager<T>::ItemsBaseAction::ItemsBaseAction(BaseManager* m, Array<T*> iList, var data) :
	ManagerBaseAction(m, data)
{
	if (data.isVoid())
	{
		std::sort(iList.begin(), iList.end(),
			[m](const auto& lhs, const auto& rhs) { return m->items.indexOf(lhs) < m->items.indexOf(rhs); });
	}

	for (auto& i : iList)
	{
		BaseItem* bi = dynamic_cast<BaseItem*>(i);
		itemsRef.add(bi);
		itemsShortName.add(bi != nullptr ? bi->shortName : "");
	}
}

template<class T>
Array<T*> BaseManager<T>::ItemsBaseAction::getItems()
{
	Array<T*> iList;
	int index = 0;
	for (auto& i : itemsRef)
	{
		if (i != nullptr && !i.wasObjectDeleted())
		{
			T* ti = dynamic_cast<T*>(i.get());
			if (ti != nullptr) iList.add(ti);
			else
			{
				BaseManager* m = this->getManager();
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
BaseManager<T>::AddItemsAction::AddItemsAction(BaseManager* m, Array<T*> iList, var data) : ItemsBaseAction(m, iList, data)
{
}

template<class T>
bool BaseManager<T>::AddItemsAction::perform()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	Array<T*> iList = this->getItems();
	if (!iList.isEmpty()) m->addItems(iList, this->data, false);
	else if (!this->data.isVoid())
	{
		Array<T*> newList = m->addItemsFromData(this->data, false);
		this->itemsRef.clear();
		for (auto& i : newList) this->itemsRef.add(i);
	}


	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}

template<class T>
bool BaseManager<T>::AddItemsAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr)
	{
		this->itemsRef.clear();
		return false;
	}

	Array<T*> iList = this->getItems();
	this->data = var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->items.indexOf(i));
			this->data.append(d);
		}
	}
	m->removeItems(iList, false);
	return true;
}

template<class T>
BaseManager<T>::RemoveItemsAction::RemoveItemsAction(BaseManager* m, Array<T*> iList) : ItemsBaseAction(m, iList) {
}

template<class T>
bool BaseManager<T>::RemoveItemsAction::perform()
{
	BaseManager* m = this->getManager();
	if (m == nullptr)
	{
		this->itemsRef.clear();
		return false;
	}

	Array<T*> iList = this->getItems();
	this->data = var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->items.indexOf(i));
			this->data.append(d);
		}
	}

	m->removeItems(iList, false);
	return true;
}

template<class T>
bool BaseManager<T>::RemoveItemsAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	Array<T*> iList = m->addItemsFromData(this->data, false);

	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}

template<class T>
BaseManager<T>::ManagerItemComparator::ManagerItemComparator(BaseManager* manager) : m(manager), compareFunc(nullptr)
{
	compareFunc = nullptr;
}

template<class T>
int BaseManager<T>::ManagerItemComparator::compareElements(ControllableContainer* i1, ControllableContainer* i2)
{
	jassert(compareFunc != nullptr);
	return compareFunc(static_cast<T*>(i1), static_cast<T*>(i2));
}
