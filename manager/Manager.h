/*
  ==============================================================================

	Manager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class T> class ItemGroup;

template <class T, class G>
class Manager :
	public EnablingControllableContainer
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	static_assert(std::is_base_of<BaseItem, G>::value, "G must be derived from BaseItem");

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

	juce::Array<BaseItem*, juce::CriticalSection> items;

	//Factory
	Factory<T, G>* managerFactory;

	virtual void clear() override;

	//Adding
	bool canAddItemOfType(const juce::String& typeToCheck);

	virtual T* createItem(); //to override if special constructor to use
	virtual BaseItem* createItemFromData(juce::var data);
	BaseItem* addItem(BaseItem* item = nullptr, juce::var data = juce::var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data
	juce::Array<BaseItem*> addItems(juce::Array<BaseItem*> items, juce::var data = juce::var(), bool addToUndo = true, bool notify = true);
	void addItemManagerInternal(BaseItem* item, juce::var data = juce::var());
	void addItemsManagerInternal(juce::Array<BaseItem*> item, juce::var data = juce::var());

	virtual BaseItem* createItemFromData(juce::var data) = 0;
	virtual BaseItem* addItemFromData(juce::var data, bool addToUndo = true);
	virtual juce::Array<BaseItem*> addItemsFromData(juce::var data, bool addToUndo = true);
	virtual juce::Array<BaseItem*> addItemsFromClipboard(bool showWarning = true);

	//Removing

	BaseItem* removeItem(BaseItem* item, bool addToUndo = true, bool notify = true, bool returnItem = false);
	void removeItems(juce::Array<BaseItem*> items, bool addToUndo = true, bool notify = true);


	void removeItemManagerInternal(BaseItem* item);
	void removeItemsManagerInternal(juce::Array<BaseItem*> item);

	//Undo
	virtual juce::UndoableAction* getAddBaseItemUndoableAction(BaseItem* item = nullptr, juce::var data = juce::var());
	virtual juce::UndoableAction* getAddBaseItemsUndoableAction(juce::Array<BaseItem*> item = nullptr, juce::var data = juce::var());
	virtual juce::Array<juce::UndoableAction*> getRemoveBaseItemUndoableAction(BaseItem* item);
	virtual juce::Array<juce::UndoableAction*> getRemoveBaseItemsUndoableAction(juce::Array<BaseItem*> items);


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
	virtual juce::Array<BaseItem*> getAllItems(bool recursive = true, bool includeDisabled = true, bool includeGroups = true) const;
	virtual juce::Array<T*> getItems(bool includeDisabled = true, bool recursive = true);
	virtual juce::Array<G*> getGroups(bool includeDisabled = true, bool recursive = true);

	virtual juce::String getItemPath(BaseItem* item);

	virtual BaseItem* getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo = false, bool searchWithLowerCaseIfNotFound = true, bool recursive = true);
	virtual BaseItem* getItemWithPath(const juce::String& relativePath);

	void callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func);


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


	typedef ManagerTListener<T, G> ManagerListener;

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

	template<class IType>
	juce::Array<IType*> getItemsWithType(bool recursive = true)
	{
		juce::Array<BaseItem*> itemsToSearch = getAllitems(recursive, true, false);
		juce::Array<IType*> result;
		for (auto& i : itemsToSearch)
		{
			if (IType* it = dynamic_cast<IType*>(i)) result.add(it);
		}

		return result;
	}

	juce::Array<T*> getAsItems(juce::Array<BaseItem*> this->items) const
	{
		juce::Array<T*> result;
		for (auto& i : this->items) if (T* it = dynamic_cast<T*>(i)) result.add(it);
		return result;
	}

	juce::Array<G*> getAsGroups(juce::Array<BaseItem*> this->items) const
	{
		juce::Array<G*> result;
		for (auto& i : this->items) if (G* it = dynamic_cast<G*>(i)) result.add(it);
		return result;
	}

	T* getAsItem(BaseItem* baseItem) const
	{
		if (baseItem == nullptr) return nullptr;
		return dynamic_cast<T*>(baseItem);
	}

	G* getAsGroup(BaseItem* baseItem) const
	{
		if (baseItem == nullptr) return nullptr;
		return dynamic_cast<G*>(baseItem);
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
			this.this->items.addArray(items);
		}

		~ManagerEvent() {}

		bool isGroup() const {
			if (this->items.isEmpty()) return false;
			if (this->items.getFirst().wasObjectDeleted()) return false;
			return this->items.getFirst()->isGroup;
		}

		juce::Array<T*> getItems() const;
		T* getItem(int index = 0) const;

		juce::Array<G*> getGroups() const;
		G* getGroup(int index = 0) const;

		juce::Array<BaseItem*> getItems() const
		{
			juce::Array<BaseItem*> items;
			for (auto& i : this->items)
			{
				if (i == nullptr) continue;
				if (i.wasObjectDeleted()) continue;
				items.add(i);
			}
			return items;
		}
	};

	using ManagerNotifier = QueuedNotifier<ManagerEvent>;
	ManagerNotifier managerNotifier;
	ManagerNotifier recursiveManagerNotifier;
	typedef typename QueuedNotifier<ManagerEvent>::Listener AsyncListener;

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

	virtual void notifyAsync(ManagerEvent::Type type, juce::Array<BaseItem*> items = juce::Array<BaseItem*>(), bool fromChildGroup = false);


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Manager);

	void askForPaste();
	void askForSelectAllItems(bool addToSelection = false);
	void askForRemoveBaseItem(BaseItem* item);
	void askForDuplicateItem(BaseItem* item);
	void askForMoveBefore(BaseItem*);
	void askForMoveAfter(BaseItem*);
	void askForSelectPreviousItem(BaseItem* item, bool addToSelection = false);
	void askForSelectNextItem(BaseItem* item, bool addToSelection = false);

	void onContainerParameterChanged(Parameter* p) override;


	virtual void loadJSONDataInternal(juce::var data) override;
	virtual void loadJSONDataManagerInternal(juce::var data);

	virtual void addItemManagerInternal(BaseItem* item, juce::var data) {}
	virtual void addItemsManagerInternal(juce::Array<BaseItem*> items, juce::var data) {}
	virtual void removeItemManagerInternal(BaseItem* item) {}
	virtual void removeItemsManagerInternal(juce::Array<BaseItem*> items) {}

	//notify, to override by templated classes
	virtual void notifyItemAdded(BaseItem* item, bool fromChildGroup = false) {}
	virtual void notifyItemsAdded(juce::Array<BaseItem*> items, bool fromChildGroup = false) {}
	virtual void notifyItemRemoved(BaseItem* item, bool fromChildGroup = false) {}
	virtual void notifyItemsRemoved(juce::Array<BaseItem*> items, bool fromChildGroup = false) {}
	virtual void notifyItemsReordered(bool fromChildGroup = false) {}

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

		juce::Array<juce::WeakReference<BaseItem>> this->items;
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


template<class T, class G>
Manager<T, G>::Manager(const juce::String& name, bool canHaveGroups) :
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
	snapGridSize(nullptr)
{
	canBeCopiedAndPasted = true;
	skipLabelInTarget = true; //by default manager label in targetParameter UI are not interesting
	nameCanBeChangedByUser = false;

	managerFactory(nullptr),
		managerNotifier(50),
		recursiveManagerNotifier(50),
		comparator(this)

		scriptObject.getDynamicObject()->setMethod("addItem", &Manager<T, G>::addItemFromScript);
	scriptObject.getDynamicObject()->setMethod("removeItem", &Manager<T, G>::removeItemFromScript);
	scriptObject.getDynamicObject()->setMethod("removeAll", &Manager<T, G>::removeAllItemsFromScript);
	scriptObject.getDynamicObject()->setMethod("getItems", &Manager<T, G>::getItemsFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemWithName", &Manager<T, G>::getItemWithNameFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemAt", &Manager<T, G>::getItemAtFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemIndex", &Manager<T, G>::getItemIndexFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemBefore", &Manager<T, G>::getItemBeforeFromScript);
	scriptObject.getDynamicObject()->setMethod("getItemAfter", &Manager<T, G>::getItemAfterFromScript);
	scriptObject.getDynamicObject()->setMethod("reorderItems", &Manager<T, G>::reorderItemsFromScript);

}


template<class T, class G>
void Manager<T, G>::clear()
{
	isClearing = true;
	//const ScopedLock lock(this->items.getLock());
	while (this->items.size() > 0) removeItem(this->items[0], false);
	isClearing = false;

#if ORGANICUI_USE_WEBSERVER
	if (notifyRemoteControlOnClear && !isCurrentlyLoadingData && isAttachedToRoot()) OSCRemoteControl::getInstance()->sendPathChangedFeedback(getControlAddress());
#endif
}

template<class T, class G>
BaseItem* Manager<T, G>::addItem(BaseItem* item, juce::var data, bool addToUndo, bool notify)
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

template<class T, class G>
juce::Array<BaseItem*> Manager<T, G>::addItems(juce::Array<BaseItem*> itemsToAdd, juce::var data, bool addToUndo, bool notify)
{
	bool curIsLoadingData = isCurrentlyLoadingData;
	bool curIsManipulatingMultipleItems = isManipulatingMultipleItems;

	isCurrentlyLoadingData = true;
	isManipulatingMultipleItems = true;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		AddItemsAction* a = new AddItemsAction(this, itemsToAdd, data);
		UndoMaster::getInstance()->performAction("Add " + String(itemsToAdd.size()) + " items", a);
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
		Array<Inspectable*> select;
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
template<class T, class G>
BaseItem* Manager<T, G>::addItemFromData(var data, bool addToUndo)
{
	BaseItem* item = createItemFromData(data);
	if (item == nullptr) return nullptr;
	return addItem(item, data, addToUndo, true);
}

template<class T, class G>
juce::Array<BaseItem*> Manager<T, G>::addItemsFromData(var data, bool addToUndo)
{
	Array<BaseItem*> itemsToAdd;

	var itemsData; //avoid offset between items array and data array if items are skipped because they're null
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

template<class T, class G>
juce::Array<BaseItem*> Manager<T, G>::addItemsFromClipboard(bool showWarning)
{
	if (!userCanAddItemsManually) return Array<BaseItem*>();
	String s = SystemClipboard::getTextFromClipboard();
	var data = JSON::parse(s);
	if (data.isVoid()) return Array<BaseItem*>();

	if (!data.hasProperty("itemType"))
	{
		Array<BaseItem*> result;
		result.add(this->addItemFromData(data));
		return result;
	}

	String t = data.getProperty("itemType", "");
	if (!canAddItemOfType(t))
	{
		if (showWarning) NLOGWARNING(niceName, "Can't paste data from clipboard : data is of wrong type (\"" + t + "\").");
		return Array<BaseItem*>();
	}

	var itemsData = data.getProperty("items", juce::var());
	int sIndex = this->items.indexOf(InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>());
	if (sIndex >= 0)
	{
		sIndex++;
		for (int i = 0; i < itemsData.size(); i++)
		{
			itemsData[i].getDynamicObject()->setProperty("index", sIndex++);
		}
	}

	Array<BaseItem*> copiedItems = addItemsFromData(itemsData);

	if (!clipboardCopyOffset.isOrigin())
	{
		for (auto& i : copiedItems)
		{
			((BaseItem*)i)->viewUIPosition->setPoint(((BaseItem*)i)->viewUIPosition->getPoint() + clipboardCopyOffset);
		}
	}

	return copiedItems;
}


template<class T, class G>
bool Manager<T, G>::canAddItemOfType(const String& typeToCheck)
{
	if (typeToCheck == itemDataType) return true;
	return false;
}


template<class T, class G>
void Manager<T, G>::loadItemsData(var data)
{
	if (data == juce::var()) return;
	Array<var>* itemsData = data.getProperty("items", juce::var()).getArray();
	if (itemsData == nullptr) return;

	for (auto& td : *itemsData)
	{
		String n = td.getProperty("niceName", "");
		BaseItem* i = getItemWithName(n, true);
		if (i != nullptr) i->loadJSONData(td);
	}
}

template<class T, class G>
BaseItem* Manager<T, G>::removeItem(BaseItem* item, bool addToUndo, bool notify, bool returnItem)
{
	if (item == nullptr) return nullptr;

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performActions("Remove " + item->getTypeString(), getRemoveBaseItemUndoableAction(item));
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

template<class T, class G>
void Manager<T, G>::removeItems(juce::Array<BaseItem*> itemsToRemove, bool addToUndo, bool notify)
{
	isManipulatingMultipleItems = true;
	if (addToUndo)
	{
		Array<UndoableAction*> a = getRemoveBaseItemsUndoableAction(itemsToRemove);
		UndoMaster::getInstance()->performActions("Remove " + String(itemsToRemove.size()) + " items", a);
		isManipulatingMultipleItems = false;
		return;
	}

	Array<BaseItem*> itemsRemoved;
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

template<class T, class G>
UndoableAction* Manager<T, G>::getAddBaseItemUndoableAction(BaseItem* item, juce::var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	jassert(this->items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	return new AddItemAction(this, item, data);
}

template<class T, class G>
UndoableAction* Manager<T, G>::getAddBaseItemsUndoableAction(juce::Array<BaseItem*> _items, juce::var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	if (_items.size() == 0) return nullptr;
	return new AddItemsAction(this, _items, data);
}

template<class T, class G>
juce::Array<UndoableAction*> Manager<T, G>::getRemoveBaseItemUndoableAction(BaseItem* item)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	Array<UndoableAction*> a;
	a.add(new RemoveItemAction(this, item));
	return a;
}

template<class T, class G>
juce::Array<UndoableAction*> Manager<T, G>::getRemoveBaseItemsUndoableAction(juce::Array<BaseItem*> _items)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	Array<UndoableAction*> a;
	a.add(new RemoveItemsAction(this, _items));
	return a;
}


template<class T, class G>
void Manager<T, G>::setItemIndex(BaseItem* item, int newIndex, bool addToUndo)
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


	newIndex = jlimit(0, this->items.size() - 1, newIndex);
	int index = this->items.indexOf(item);
	if (index == -1 || index == newIndex) return;

	//items.getLock().enter();
	this->items.move(index, newIndex);
	controllableContainers.move(index, newIndex);
	//items.getLock().exit();

	notifyItemsReordered();
}

template<class T, class G>
juce::Array<UndoableAction*> Manager<T, G>::getSetItemIndexUndoableAction(BaseItem* item, int newIndex)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	Array<UndoableAction*> a;
	a.add(new MoveItemAction(this, item, this->items.indexOf(item), newIndex));
	return a;
}

template<class T, class G>
BaseItem* Manager<T, G>::getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo, bool searchWithLowerCaseIfNotFound, bool recursive)
{
	//const ScopedLock lock(this->items.getLock());

	Array<BaseItem*> itemsToSearch = getAllItems(recursive, true, false);

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

template<class T, class G>
String Manager<T, G>::getItemPath(BaseItem* item)
{
	return item->getControlAddress(this);
}

template<class T, class G>
BaseItem* Manager<T, G>::getItemWithPath(const juce::String& relativePath)
{
	if (BaseItem* i = dynamic_cast<BaseItem*>(getControllableContainerForAddress(relativePath))) return i;
	return nullptr;
}



template<class T, class G>
void Manager<T, G>::callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func)
{
	for (auto& i : this->items)
	{
		if (i->canBeDisabled && (i->enabled->boolValue() || includeDisabled)) continue;

		if (i->isGroup)
		{
			if (includeGroups) func(i);
			if (recursive)
			{
				auto* group = (BaseItemGroup*)i;
				if (group) group->baseManager->callFunctionOnAllItems(recursive, includeGroups, includeDisabled, func);
			}
		}
		else
		{
			func(i);
		}
	}
}

template<class T, class G>
void Manager<T, G>::handleAddFromRemoteControl(var data)
{
	if (!userCanAddItemsManually) return;
	addItemFromData(data);
}

template<class T, class G>
void Manager<T, G>::askForRemoveBaseItem(BaseItem* item)
{
	removeItem(item);
}

template<class T, class G>
void Manager<T, G>::askForDuplicateItem(BaseItem* item)
{
	if (!userCanAddItemsManually) return;
	var data = item->getJSONData();
	data.getDynamicObject()->setProperty("index", this->items.indexOf(static_cast<BaseItem*>(item)) + 1);
	addItemFromData(data);
}

template<class T, class G>
void Manager<T, G>::askForPaste()
{
	addItemsFromClipboard();
}

template<class T, class G>
void Manager<T, G>::askForMoveBefore(BaseItem* item)
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

template<class T, class G>
void Manager<T, G>::askForMoveAfter(BaseItem* item)
{
	setItemIndex(item, jmin(this->items.indexOf(item) + 1, this->items.size() - 1));
	//int index = this->items.indexOf(static_cast<T *>(i));
	//if (index == this->items.size() -1) return;
	//this->items.swap(index, index + 1);
	//controllableContainers.swap(index, index+1);

	//managerListeners.call(&ManagerListener::itemsReordered);
	//managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T, class G>
void Manager<T, G>::askForSelectAllItems(bool addToSelection)
{
	int numItems = this->items.size();
	if (!addToSelection) selectionManager->clearSelection(numItems == 0);
	else deselectThis(numItems == 0);

	if (numItems > 1) for (int i = 0; i < numItems; ++i) this->items[i]->selectThis(true, i == numItems - 1); //only notify on last
	else if (numItems > 0) this->items.getFirst()->selectThis(addToSelection, true);
}

template<class T, class G>
void Manager<T, G>::askForSelectPreviousItem(BaseItem* item, bool addToSelection)
{
	int index = this->items.indexOf(item);
	if (index <= 0) return;
	this->items[index - 1]->selectThis(addToSelection);
}

template<class T, class G>
void Manager<T, G>::askForSelectNextItem(BaseItem* item, bool addToSelection)
{
	int index = this->items.indexOf(item);
	if (index == -1 || index >= this->items.size() - 1) return;
	this->items[index + 1]->selectThis(addToSelection);
}

template<class T, class G>
void Manager<T, G>::onContainerParameterChanged(Parameter* p)
{
	EnablingControllableContainer::onContainerParameterChanged(p);
	if (p == showSnapGrid || p == snapGridSize)
	{
		notifyAsync(ManagerEvent::NEEDS_UI_UPDATE);
	}
}

template<class T, class G>
var Manager<T, G>::getExportSelectionData()
{
	var data;

	for (auto& t : this->items)
	{
		if (t->isSavable && t->isSelected) data.append(t->getJSONData());
	}

	return data;
}

template<class T, class G>
var Manager<T, G>::getJSONData(bool includeNonOverriden)
{
	var data = ControllableContainer::getJSONData(includeNonOverriden);
	var itemsData = juce::var();
	//this->items.getLock().enter();
	for (auto& t : this->items)
	{
		if (t->isSavable) itemsData.append(t->getJSONData(includeNonOverriden));
	}
	//this->items.getLock().exit();

	if (itemsData.size() > 0) data.getDynamicObject()->setProperty("items", itemsData);

	var vData;
	vData.append(viewOffset.x);
	vData.append(viewOffset.y);
	data.getDynamicObject()->setProperty("viewOffset", vData);
	data.getDynamicObject()->setProperty("viewZoom", viewZoom);

	return data;
}
template<class T, class G>
void Manager<T, G>::loadJSONDataInternal(var data)
{
	clear();

	if (data.hasProperty("viewOffset"))
	{
		var vData = data.getProperty("viewOffset", juce::var());
		viewOffset.setXY(vData[0], vData[1]);
	}

	viewZoom = data.getProperty("viewZoom", 1);

	loadJSONDataManagerInternal(data);
}
template<class T, class G>
void Manager<T, G>::loadJSONDataManagerInternal(var data)
{
	var itemsData = data.getProperty("items", juce::var());
	if (itemsData.isVoid()) return;

	addItemsFromData(itemsData, false);
}

template<class T, class G>
void Manager<T, G>::getRemoteControlDataInternal(var& data)
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

template<class T, class G>
String Manager<T, G>::getScriptTargetString()
{
	return "[" + niceName + " : Manager(" + itemDataType + ")]";
}

template<class T, class G>
Manager<T, G>::ManagerEvent::ManagerEvent(Type t, Array<BaseItem*> iList, bool fromChildGroup) :
	type(t),
	fromChildGroup(fromChildGroup)
{
	for (auto& i : iList) if (i != nullptr) this->items.add(i);
}


//ACTIONS
template<class T, class G>
Manager<T, G>::ManagerBaseAction::ManagerBaseAction(Manager<T, G>* manager, juce::var _data) :
	managerControlAddress(manager->getControlAddress()),
	data(_data),
	managerRef(manager)
{
}

template<class T, class G>
Manager<T, G>* Manager<T, G>::ManagerBaseAction::getManager() {
	if (managerRef != nullptr && !managerRef.wasObjectDeleted()) return dynamic_cast<Manager*>(managerRef.get());
	else if (Engine::mainEngine != nullptr)
	{
		ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(managerControlAddress, true);
		if (cc != nullptr) return dynamic_cast<Manager*>(cc);
	}

	return nullptr;
}

template<class T, class G>
Manager<T, G>::ItemBaseAction::ItemBaseAction(Manager* m, BaseItem* i, juce::var data) :
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

template<class T, class G>
BaseItem* Manager<T, G>::ItemBaseAction::getItem()
{
	if (baseItem != nullptr && !baseItem.wasObjectDeleted()) return baseItem.get();
	else
	{
		Manager* m = this->getManager();
		if (m != nullptr) return m->getItemWithName(itemShortName);
	}

	return nullptr;
}

template<class T, class G>
Manager<T, G>::AddItemAction::AddItemAction(Manager* m, BaseItem* i, juce::var data) : ItemBaseAction(m, i, data) {
}

template<class T, class G>
bool Manager<T, G>::AddItemAction::perform()
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

template<class T, class G>
bool Manager<T, G>::AddItemAction::undo()
{
	BaseItem* s = this->getItem();
	if (s == nullptr) return false;
	this->data = s->getJSONData();
	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(s, false);
	this->baseItem = nullptr;
	return true;
}

template<class T, class G>
Manager<T, G>::RemoveItemAction::RemoveItemAction(Manager* m, BaseItem* i, juce::var data) : ItemBaseAction(m, i, data)
{

}

template<class T, class G>
bool Manager<T, G>::RemoveItemAction::perform()
{

	if (this->baseItem == nullptr) return false;

	this->data = this->baseItem->getJSONData();
	if (this->data.getDynamicObject() == nullptr) return false;

	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(this->baseItem, false);
	this->baseItem = nullptr;
	return true;
}

template<class T, class G>
bool Manager<T, G>::RemoveItemAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;
	this->baseItem = m->addItemFromData(this->data, false);
	return true;
}


template<class T, class G>
Manager<T, G>::MoveItemAction::MoveItemAction(Manager* m, BaseItem* i, int prevIndex, int newIndex) :
	ItemBaseAction(m, i),
	prevIndex(prevIndex),
	newIndex(newIndex)
{

}

template<class T, class G>
bool Manager<T, G>::MoveItemAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	BaseItem* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, newIndex, false);
	return true;
}

template<class T, class G>
bool Manager<T, G>::MoveItemAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	BaseItem* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, prevIndex, false);
	return true;
}


template<class T, class G>
Manager<T, G>::ItemsBaseAction::ItemsBaseAction(Manager* m, Array<BaseItem*> iList, juce::var data) :
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

template<class T, class G>
juce::Array<BaseItem*> Manager<T, G>::ItemsBaseAction::getItems()
{
	Array<BaseItem*> iList;
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

template<class T, class G>
Manager<T, G>::AddItemsAction::AddItemsAction(Manager* m, Array<BaseItem*> iList, juce::var data) :
	ItemsBaseAction(m, iList, data)
{
}

template<class T, class G>
bool Manager<T, G>::AddItemsAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	Array<BaseItem*> iList = this->getItems();
	if (!iList.isEmpty()) m->addItems(iList, this->data, false);
	else if (!this->data.isVoid())
	{
		Array<BaseItem*> newList = m->addItemsFromData(this->data, false);
		this->items.clear();
		for (auto& i : newList) this->items.add(i);
	}


	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}

template<class T, class G>
bool Manager<T, G>::AddItemsAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr)
	{
		this->items.clear();
		return false;
	}

	Array<BaseItem*> iList = this->getItems();
	this->data = juce::var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->this->items.indexOf(i));
			this->data.append(d);
		}
	}
	m->removeItems(iList, false);
	return true;
}

template<class T, class G>
Manager<T, G>::RemoveItemsAction::RemoveItemsAction(Manager* m, Array<BaseItem*> iList) : ItemsBaseAction(m, iList) {
}

template<class T, class G>
bool Manager<T, G>::RemoveItemsAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr)
	{
		this->items.clear();
		return false;
	}

	Array<BaseItem*> iList = this->getItems();
	this->data = juce::var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->this->items.indexOf(i));
			this->data.append(d);
		}
	}

	m->removeItems(iList, false);
	return true;
}

template<class T, class G>
bool Manager<T, G>::RemoveItemsAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	Array<BaseItem*> iList = m->addItemsFromData(this->data, false);

	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}



template<class T, class G>
void Manager<T, G>::setHasGridOptions(bool hasGridOptions)
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

template<class T, class G>
T* Manager<T, G>::createItem()
{
	if (managerFactory != nullptr && managerFactory->defs.size() == 1) return managerFactory->create(managerFactory->defs[0]);
	return new T();
}

template<class T, class G>
BaseItem* Manager<T, G>::createItemFromData(juce::var data)
{
	if (canHaveGroups)
	{
		bool isGroup = data.getProperty("type", "").toString() == "ItemGroup";
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

template<class T, class G>
void Manager<T, G>::notifyItemAdded(BaseItem* item, bool fromChildGroup)
{
	if (canHaveGroups)
	{
		if (Manager<T, G>* m = getParentAs<Manager<T, G>>(2)) m->notifyItemAdded(item, true);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	if (item->isGroup) listenersToCall->call(&ManagerListener::groupAdded, (G*)item);
	else listenersToCall->call(&ManagerListener::itemAdded, (T*)item);

	notifyAsync(ManagerEvent::ITEM_ADDED, (T*)item, fromChildGroup);
}

template<class T, class G>
void Manager<T, G>::notifyItemsAdded(juce::Array<BaseItem*> items, bool fromChildGroup)
{
	if (items.isEmpty()) return;

	if (canHaveGroups)
	{
		if (Manager<T, G>* m = getParentAs<Manager<T, G>>(2)) m->notifyItemsAdded(items, true);
	}

	juce::Array<T*> itemsToAdd;
	juce::Array<G*> groupsToAdd;
	for (auto& i : items)
	{
		if (T* it = dynamic_cast<T*>(i)) itemsToAdd.add(it);
		else if (G* it = dynamic_cast<G*>(i)) groupsToAdd.add(it);
	}
	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	if (!itemsToAdd.isEmpty()) listenersToCall->call(&ManagerListener::itemsAdded, itemsToAdd);
	if (!groupsToAdd.isEmpty()) listenersToCall->call(&ManagerListener::groupsAdded, groupsToAdd);

	notifyAsync(ManagerEvent::ITEMS_ADDED, items, fromChildGroup);
}


template<class T, class G>
void Manager<T, G>::notifyItemRemoved(BaseItem* item, bool fromChildGroup)
{
	if (canHaveGroups)
	{
		if (Manager<T, G>* m = getParentAs<Manager<T, G>>(2)) m->notifyItemRemoved(item, true);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	if (item->isGroup) listenersToCall->call(&ManagerListener::groupRemoved, (G*)item);
	else listenersToCall->call(&ManagerListener::itemRemoved, (T*)item);

	notifyAsync(ManagerEvent::ITEM_REMOVED, (T*)item, fromChildGroup);
}

template<class T, class G>
void Manager<T, G>::notifyItemsRemoved(juce::Array<BaseItem*> items, bool fromChildGroup)
{
	if (items.isEmpty()) return;

	if (canHaveGroups)
	{
		if (Manager<T, G>* m = getParentAs<Manager<T, G>>(2)) m->notifyItemsRemoved(items, true);
	}

	juce::Array<T*> itemsToRemove;
	juce::Array<G*> groupsToRemove;
	for (auto& i : items)
	{
		if (T* it = dynamic_cast<T*>(i)) itemsToRemove.add(it);
		else if (G* it = dynamic_cast<G*>(i)) groupsToRemove.add(it);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	if (!itemsToRemove.isEmpty()) listenersToCall->call(&ManagerListener::itemsRemoved, itemsToRemove);
	if (!groupsToRemove.isEmpty()) listenersToCall->call(&ManagerListener::groupsRemoved, groupsToRemove);

	notifyAsync(ManagerEvent::ITEMS_REMOVED, items, fromChildGroup);
}

template<class T, class G>
void Manager<T, G>::notifyItemsReordered(bool fromChildGroup)
{
	if (canHaveGroups)
	{
		if (Manager<T, G>* m = getParentAs<Manager<T, G>>(2)) m->notifyItemsReordered(true);
	}

	juce::ListenerList<ManagerListener>* listenersToCall = fromChildGroup ? &recursiveManagerListeners : &managerListeners;
	listenersToCall->call(&ManagerListener::itemsReordered);
	notifyAsync(ManagerEvent::ITEMS_REORDERED, juce::Array<BaseItem*>(), fromChildGroup);
}

template<class T, class G>
juce::Array<BaseItem*> Manager<T, G>::getAllItems(bool recursive, bool includeDisabled, bool includeGroups) const
{
	Array<BaseItem*> result;
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

template<class T, class G>
juce::Array<T*> Manager<T, G>::getItems(bool includeDisabled, bool recursive)
{
	return getAllItems(recursive, includeDisabled, false);
}

template<class T, class G>
juce::Array<G*> Manager<T, G>::getGroups(bool includeDisabled, bool recursive)
{
	juce::Array<BaseItem*> this->items = getAllItems(recursive, includeDisabled, true);
	juce::Array<G*> result;
	for (auto& i : this->items) if (i->isGroup) result.add(dynamic_cast<G*>(i));
	return result;
}

template<class T, class G>
juce::PopupMenu Manager<T, G>::getItemsMenu(int startID)
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

template<class T, class G>
T* Manager<T, G>::getItemForMenuResultID(int id, int startID)
{
	return items[id - startID];
}

template<class T, class G>
T* Manager<T, G>::getFirstSelectedItem()
{
	for (auto& i : items) if (i->isSelected) return i;
	return nullptr;
}

template<class T, class G>
void Manager<T, G>::notifyAsync(ManagerEvent::Type type, juce::Array<BaseItem*> _items, bool fromChildGroup)
{
	ManagerNotifier* notifierToCall = fromChildGroup ? &recursiveManagerNotifier : &this->managerNotifier;
	notifierToCall->addMessage(new ManagerEvent(type, _items, fromChildGroup));
}


template<class T, class G>
InspectableEditor* Manager<T, G>::getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables)
{
	return new GenericManagerEditor<T, G>(this, isRoot);
}


//SCRIPT
template<class T, class G>
juce::var Manager<T, G>::addItemFromScript(const juce::var::NativeFunctionArgs& args)
{
	Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args);

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
			T* item = m->managerFactory->create(args.arguments[0].toString());
			m->addItem(item, args.numArguments > 1 && args.arguments[1].isObject() ? args.arguments[1] : juce::var());
			if (item != nullptr) return item->getScriptObject();
		}
	}

	return juce::var();
}


template<class T, class G>
juce::var Manager<T, G>::removeItemFromScript(const juce::var::NativeFunctionArgs& args)
{
	Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args);

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
	else if (args.arguments[0].isString())
	{
		T* item = m->getItemWithName(args.arguments[0].toString(), true);
		if (item != nullptr)
		{
			m->removeItem(item);
			return juce::var();
		}
	}

	NLOGWARNING(m->niceName, "Remove item : item not found in manager");
	return juce::var();
}

template<class T, class G>
juce::var Manager<T, G>::removeAllItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		m->clear();
	}

	return juce::var();
}

template<class T, class G>
juce::var Manager<T, G>::getItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	juce::var result = juce::var();
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		for (auto& i : m->items) result.append(i->getScriptObject());
	}

	return result;
}

template<class T, class G>
juce::var Manager<T, G>::getItemWithNameFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		T* i = m->getItemWithName(args.arguments[0].toString(), true, true);
		if (i != nullptr) return i->getScriptObject();
	}

	return juce::var();
}

template<class T, class G>
juce::var Manager<T, G>::getItemAtFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		int index = args.arguments[0];
		if (index < 0 || index >= m->items.size()) return juce::var();
		T* i = m->items[index];
		if (i != nullptr) return i->getScriptObject();
	}

	return juce::var();
}

template<class T, class G>
juce::var Manager<T, G>::getItemIndexFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		if (!args.arguments[0].isObject()) return juce::var();
		T* item = dynamic_cast<T*>((T*)(juce::int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return juce::var();
		return m->items.indexOf(item);
	}

	return juce::var();
}

template<class T, class G>
juce::var Manager<T, G>::getItemBeforeFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
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

template<class T, class G>
juce::var Manager<T, G>::getItemAfterFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
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

template<class T, class G>
juce::var Manager<T, G>::reorderItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		m->reorderItems();
	}

	return juce::var();
}


//MANAGER EVENT
template<class T, class G>
juce::Array<T*> Manager<T, G>::ManagerEvent::getItems() const
{
	juce::Array<T*> result;
	for (auto& i : this->items)
	{
		if (i != nullptr && !i.wasObjectDeleted() && !i->isGroup) result.add(dynamic_cast<T*>(i.get()));
	}
	return result;
}

template<class T, class G>
T* Manager<T, G>::ManagerEvent::getItem(int index) const
{
	if (this->items.size() > index && this->items[index] != nullptr && !this->items[index].wasObjectDeleted()) return static_cast<T*>(this->items[index].get());
	return nullptr;
}

template<class T, class G>
juce::Array<G*> Manager<T, G>::ManagerEvent::getGroups() const
{
	juce::Array<G*> result;
	for (auto& i : this->items)
	{
		if (i != nullptr && !i.wasObjectDeleted()) result.add(dynamic_cast<G*>(i.get()));
	}
	return result;
}

template<class T, class G>
G* Manager<T, G>::ManagerEvent::getGroup(int index) const
{
	if (this->items.size() > index && this->items[index] != nullptr && !this->items[index].wasObjectDeleted()) return static_cast<G*>(this->items[index].get());
	return nullptr;
}


template<class T, class G>
Manager<T, G>::ManagerItemComparator::ManagerItemComparator(Manager* manager) : m(manager), compareFunc(nullptr)
{
	compareFunc = nullptr;
}

template<class T, class G>
int Manager<T, G>::ManagerItemComparator::compareElements(ControllableContainer* i1, ControllableContainer* i2)
{
	jassert(compareFunc != nullptr);
	return compareFunc(static_cast<T*>(i1), static_cast<T*>(i2));
}