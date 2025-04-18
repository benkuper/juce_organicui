/*
  ==============================================================================

	Manager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template <typename T> class GenericManagerEditor;
template <typename T> class ItemGroup;

template <typename T, typename G = ItemGroup<T>>
class Manager :
	public BaseManager
{
public:

	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	static_assert(std::is_base_of<ItemGroup<T>, G>::value, "G must be derived from ItemGroup<T>");

	Manager(const juce::String& name);
	virtual ~Manager();

	//Factory
	Factory<T>* managerFactory;
	juce::String itemDataType;
	std::function<T* ()> customCreateItemFunc;


	virtual T* createItem(); //to override if special constructor to use
	virtual T* createItemFromData(juce::var data); //to be overriden for specific item creation (from data)
	virtual T* addItemFromData(juce::var data, bool addToUndo = true); //to be overriden for specific item creation (from data)
	virtual juce::Array<T*> addItemsFromData(juce::var data, bool addToUndo = true); //to be overriden for specific item creation (from data)
	virtual juce::Array<T*> addItemsFromClipboard(bool showWarning = true);
	virtual bool canAddItemOfType(const juce::String& typeToCheck);

	virtual void loadItemsData(juce::var data);

	virtual juce::UndoableAction* getAddItemUndoableAction(T* item = nullptr, juce::var data = juce::var());
	virtual juce::UndoableAction* getAddItemsUndoableAction(juce::Array<T*> item = nullptr, juce::var data = juce::var());

	T* addItem(T* item = nullptr, juce::var data = juce::var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data
	T* addItem(const juce::Point<float> initialPosition, bool addToUndo = true, bool notify = true);
	T* addItem(T* item, const juce::Point<float> initialPosition, bool addToUndo = true, bool notify = true);
	juce::Array<T*> addItems(juce::Array<T*> items, juce::var data = juce::var(), bool addToUndo = true);


	virtual juce::Array<juce::UndoableAction*> getRemoveItemUndoableAction(T* item);
	virtual juce::Array<juce::UndoableAction*> getRemoveItemsUndoableAction(juce::Array<T*> items);

	void removeItems(juce::Array<T*> items, bool addToUndo = true);
	T* removeItem(T* item, bool addToUndo = true, bool notify = true, bool returnItem = false);

	virtual void setItemIndex(T* item, int newIndex, bool addToUndo = true);
	virtual juce::Array<juce::UndoableAction*> getSetItemIndexUndoableAction(T* item, int newIndex);

	virtual void reorderItems(); //to be overriden if needed


	//to override for specific handling like adding custom listeners, etc.
	virtual void addItemInternal(T*, juce::var data) {}
	virtual void addItemsInternal(juce::Array<T*>, juce::var data) {}
	virtual void removeItemInternal(T*) {}
	virtual void removeItemsInternal(juce::Array<T*>) {}

	virtual T* getItemAt(int index) const;
	virtual T* getFirstItem(bool recursive = false) const;
	virtual T* getLastItem(bool recursive = false) const;

	virtual T* getItemBefore(T* item) const;
	virtual T* getItemAfter(T* item) const;

	virtual juce::Array<T*> getItems(bool recursive = false) const;
	virtual juce::Array<G*> getItemGroups(bool recursive = true) const;
	T* getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo = false, bool searchWithLowerCaseIfNotFound = true, bool recursive = true);

	template<typename IType>
	juce::Array<IType*> getItemsWithType(bool recursive = true);

	void callFunctionOnItems(std::function<void(T*)> func, bool recursive = false);
	bool callStoppingFunctionOnItems(std::function<bool(T*)> func, bool recursive = false);

	virtual void handleAddFromRemoteControl(juce::var data) override;


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

	virtual juce::var getExportSelectionData();
	virtual juce::var getJSONData(bool includeNonOverriden = false) override;
	virtual void loadJSONDataInternal(juce::var data) override;
	virtual void loadJSONDataManagerInternal(juce::var data);

	virtual juce::var getRemoteControlData() override;
	virtual void getRemoteControlDataInternal(juce::var& data) override;

	juce::PopupMenu getItemsMenu(int startID);
	T* getItemForMenuResultID(int id, int startID);

	T* getFirstSelectedItem();

	juce::String getScriptTargetString() override;

	typedef ManagerTListener<T> ManagerListener;

	juce::ListenerList<ManagerListener> managerListeners;
	void addManagerListener(ManagerListener* newListener) { managerListeners.add(newListener); }
	void removeManagerListener(ManagerListener* listener) {
		if (isBeingDestroyed) return;
		managerListeners.remove(listener);
	}

	class ManagerEvent
	{
	public:
		enum Type { ITEM_ADDED, ITEM_REMOVED, ITEMS_REORDERED, ITEMS_ADDED, ITEMS_REMOVED, MANAGER_CLEARED, NEEDS_UI_UPDATE };

		ManagerEvent(Type t, T* i = nullptr);
		ManagerEvent(Type t, juce::Array<T*> iList);

		Type type;
		juce::Array<juce::WeakReference<Inspectable>> itemsRef;
		juce::Array<T*> getItems() const;
		T* getItem(int index = 0) const;
	};

	using BManagerEvent = typename Manager<T, G>::ManagerEvent;
	using ManagerNotifier = QueuedNotifier<BManagerEvent>;
	ManagerNotifier managerNotifier;
	typedef typename QueuedNotifier<BManagerEvent>::Listener AsyncListener;

	void addAsyncManagerListener(AsyncListener* newListener) { managerNotifier.addListener(newListener); }
	void addAsyncCoalescedManagerListener(AsyncListener* newListener) { managerNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncManagerListener(AsyncListener* listener) { managerNotifier.removeListener(listener); }


	InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;

	//UNDO MANAGER
	class ManagerBaseAction :
		public juce::UndoableAction
	{
	public:
		ManagerBaseAction(Manager* manager, juce::var _data = juce::var());

		juce::String managerControlAddress;
		juce::var data;
		juce::WeakReference<Inspectable> managerRef;

		Manager<T, G>* getManager();
	};

	class ItemBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemBaseAction(Manager* m, T* i, juce::var data = juce::var());

		juce::WeakReference<Inspectable> itemRef;
		juce::String itemShortName;
		int itemIndex;

		T* getItem();
	};

	class AddItemAction :
		public ItemBaseAction
	{
	public:
		AddItemAction(Manager* m, T* i, juce::var data = juce::var());

		bool perform() override;
		bool undo() override;
	};

	class RemoveItemAction :
		public ItemBaseAction
	{
	public:
		RemoveItemAction(Manager* m, T* i, juce::var data = juce::var());

		bool perform() override;
		bool undo() override;
	};

	class MoveItemAction :
		public ItemBaseAction
	{
	public:
		MoveItemAction(Manager* m, T* i, int prevIndex, int newIndex);

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
		ItemsBaseAction(Manager* m, juce::Array<T*> iList, juce::var data = juce::var());

		juce::Array<juce::WeakReference<Inspectable>> itemsRef;
		juce::StringArray itemsShortName;

		juce::Array<T*> getItems();
	};

	class AddItemsAction :
		public ItemsBaseAction
	{
	public:
		AddItemsAction(Manager* m, juce::Array<T*> iList, juce::var data = juce::var());

		int startIndex;
		bool perform() override;
		bool undo() override;
	};


	class RemoveItemsAction :
		public ItemsBaseAction
	{
	public:
		RemoveItemsAction(Manager* m, juce::Array<T*> iList);

		bool perform() override;
		bool undo() override;
	};
	

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

private:
	juce::Array<T*, juce::CriticalSection> items;
	juce::Array<ItemGroup<T>*, juce::CriticalSection> itemGroups;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Manager);
};


template<typename T, typename G>
Manager<T, G>::Manager(const juce::String& name) :
	BaseManager(name),
	managerFactory(nullptr),
	customCreateItemFunc(nullptr),
	managerNotifier(50)
{

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

template<typename T, typename G>
Manager<T, G>::~Manager()
{
	clear();
}


template<typename T, typename G>
T* Manager<T, G>::createItem()
{
	if (managerFactory != nullptr && managerFactory->defs.size() == 1) return managerFactory->create(managerFactory->defs[0]);
	if (customCreateItemFunc != nullptr) return customCreateItemFunc();
	return new T();
}

template<typename T, typename G>
T* Manager<T, G>::createItemFromData(juce::var data)
{
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

template<typename T, typename G>
juce::UndoableAction* Manager<T, G>::getAddItemUndoableAction(T* item, juce::var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	jassert(items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	return new AddItemAction(this, item, data);
}

template<typename T, typename G>
juce::UndoableAction* Manager<T, G>::getAddItemsUndoableAction(juce::Array<T*> _items, juce::var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	if (_items.size() == 0) return nullptr;
	return new AddItemsAction(this, _items, data);
}

template<typename T, typename G>
T* Manager<T, G>::addItem(T* item, juce::var data, bool addToUndo, bool notify)
{
	jassert(items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	if (item == nullptr) return nullptr; //could not create here

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performAction("Add " + item->niceName, new AddItemAction(this, item, data));
			return item;
		}
	}

	int targetIndex = data.getProperty("index", -1);
	if (targetIndex != -1)
	{
		baseItems.insert(targetIndex, item);
		items.insert(targetIndex, item);
	}
	else
	{
		if (autoReorderOnAdd && !isCurrentlyLoadingData && comparator.compareFunc != nullptr) baseItems.addSorted(comparator, item);
		else
		{
			baseItems.add(item);
			items.add(item);
		}
	}

	item->addBaseItemListener(this);

	if (!data.isVoid())
	{
		item->loadJSONData(data);
	}

	addChildControllableContainer(item, false, baseItems.indexOf(item), notify);

	addItemInternal(item, data);

	if (notify)
	{
		managerListeners.call(&ManagerListener::itemAdded, item);
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_ADDED, item));

	}

	if (juce::MessageManager::getInstance()->existsAndIsLockedByCurrentThread())
	{
		if (selectItemWhenCreated && !isCurrentlyLoadingData && !isManipulatingMultipleItems) item->selectThis();
	}

	return item;
}

template<typename T, typename G>
T* Manager<T, G>::addItem(const juce::Point<float> initialPosition, bool addToUndo, bool notify)
{
	return addItem(nullptr, initialPosition, addToUndo, notify);
}

template<typename T, typename G>
T* Manager<T, G>::addItem(T* item, const juce::Point<float> initialPosition, bool addToUndo, bool notify)
{
	if (item == nullptr) item = createItem();
	item->viewUIPosition->setPoint(initialPosition);
	addItem(item, addToUndo, notify);
	return item;
}

template<typename T, typename G>
juce::Array<T*> Manager<T, G>::addItems(juce::Array<T*> itemsToAdd, juce::var data, bool addToUndo)
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

	managerListeners.call(&ManagerListener::itemsAdded, itemsToAdd);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_ADDED, itemsToAdd));

	reorderItems();
	isCurrentlyLoadingData = curIsLoadingData;
	isManipulatingMultipleItems = curIsManipulatingMultipleItems;

	if (selectItemWhenCreated && !isCurrentlyLoadingData)
	{
		juce::Array<Inspectable*> select;
		for (auto& i : itemsToAdd) select.add(i);
		selectionManager->selectInspectables(select);
	}

	addItemsInternal(itemsToAdd, data);

	return itemsToAdd;
}

//if data is not empty, load data
template<typename T, typename G>
T* Manager<T, G>::addItemFromData(juce::var data, bool addToUndo)
{
	T* item = createItemFromData(data);
	if (item == nullptr) return nullptr;
	return addItem(item, data, addToUndo);
}

template<typename T, typename G>
juce::Array<T*> Manager<T, G>::addItemsFromData(juce::var data, bool addToUndo)
{
	juce::Array<T*> itemsToAdd;

	juce::var itemsData; //avoid offset between items array and data array if items are skipped because they're null
	for (int i = 0; i < data.size(); i++)
	{
		if (T* item = createItemFromData(data[i]))
		{
			itemsToAdd.add(item);
			itemsData.append(data[i]);
		}
	}
	return addItems(itemsToAdd, itemsData, addToUndo);
}

template<typename T, typename G>
juce::Array<T*> Manager<T, G>::addItemsFromClipboard(bool showWarning)
{
	if (!userCanAddItemsManually) return juce::Array<T*>();
	juce::String s = juce::SystemClipboard::getTextFromClipboard();
	juce::var data = juce::JSON::parse(s);
	if (data.isVoid()) return juce::Array<T*>();

	if (!data.hasProperty("itemType"))
	{
		juce::Array<T*> result;
		result.add(this->addItemFromData(data));
		return result;
	}

	juce::String t = data.getProperty("itemType", "");
	if (!canAddItemOfType(t))
	{
		if (showWarning) NLOGWARNING(niceName, "Can't paste data from clipboard : data is of wrong type (\"" + t + "\").");
		return juce::Array<T*>();
	}

	juce::var itemsData = data.getProperty("items", juce::var());
	int sIndex = baseItems.indexOf(InspectableSelectionManager::activeSelectionManager->getInspectableAs<T>());
	if (sIndex >= 0)
	{
		sIndex++;
		for (int i = 0; i < itemsData.size(); i++)
		{
			itemsData[i].getDynamicObject()->setProperty("index", sIndex++);
		}
	}

	juce::Array<T*> copiedItems = addItemsFromData(itemsData);

	if (!clipboardCopyOffset.isOrigin())
	{
		for (auto& i : copiedItems)
		{
			((BaseItem*)i)->viewUIPosition->setPoint(((BaseItem*)i)->viewUIPosition->getPoint() + clipboardCopyOffset);
		}
	}

	return copiedItems;
}

template<typename T, typename G>
bool Manager<T, G>::canAddItemOfType(const juce::String& typeToCheck)
{
	if (typeToCheck == itemDataType) return true;
	if (this->managerFactory != nullptr && this->managerFactory->hasDefinitionWithType(typeToCheck)) return true;

	return false;
}

template<typename T, typename G>
void Manager<T, G>::loadItemsData(juce::var data)
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

template<typename T, typename G>
juce::Array<juce::UndoableAction*> Manager<T, G>::getRemoveItemUndoableAction(T* item)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	juce::Array<juce::UndoableAction*> a;
	a.add(new RemoveItemAction(this, item));
	return a;
}

template<typename T, typename G>
juce::Array<juce::UndoableAction*> Manager<T, G>::getRemoveItemsUndoableAction(juce::Array<T*> itemsToRemove)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	juce::Array<juce::UndoableAction*> a;
	a.add(new RemoveItemsAction(this, itemsToRemove));
	return a;
}

template<typename T, typename G>
void Manager<T, G>::removeItems(juce::Array<T*> itemsToRemove, bool addToUndo)
{
	isManipulatingMultipleItems = true;
	if (addToUndo)
	{
		juce::Array<juce::UndoableAction*> a = getRemoveItemsUndoableAction(itemsToRemove);
		UndoMaster::getInstance()->performActions("Remove " + juce::String(itemsToRemove.size()) + " items", a);
		isManipulatingMultipleItems = false;
		return;
	}

	juce::Array<T*> itemsRemoved;
	for (auto& i : itemsToRemove) itemsRemoved.add(removeItem(i, false, false, true));

	removeItemsInternal(itemsRemoved);

	managerListeners.call(&ManagerListener::itemsRemoved, itemsRemoved);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REMOVED));

	for (auto& i : itemsRemoved)
	{
		((BaseItem*)i)->clearItem();
		delete i;
	}

	isManipulatingMultipleItems = false;
}

template<typename T, typename G>
T* Manager<T, G>::removeItem(T* item, bool addToUndo, bool notify, bool returnItem)
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


	//baseItems.getLock().enter();
	items.removeAllInstancesOf(item);
	baseItems.removeObject(item, false);
	//baseItems.getLock().exit();

	removeItemInternal(item);

	BaseItem* bi = static_cast<BaseItem*>(item);
	bi->removeBaseItemListener(this);
	removeChildControllableContainer(bi);

	if (notify)
	{
		managerListeners.call(&ManagerListener::itemRemoved, item);
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_REMOVED, item));
	}

	if (returnItem) return item; //will need to delete !

	bi->clearItem();
	delete item;

	return nullptr;
}

template<typename T, typename G>
void Manager<T, G>::setItemIndex(T* item, int newIndex, bool addToUndo)
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


	newIndex = juce::jlimit(0, baseItems.size() - 1, newIndex);
	int index = baseItems.indexOf(item);
	if (index == -1 || index == newIndex) return;

	//baseItems.getLock().enter();
	baseItems.move(index, newIndex);
	controllableContainers.move(index, newIndex);
	//baseItems.getLock().exit();

	managerListeners.call(&ManagerListener::itemsReordered);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<typename T, typename G>
juce::Array<juce::UndoableAction*> Manager<T, G>::getSetItemIndexUndoableAction(T* item, int newIndex)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	juce::Array<juce::UndoableAction*> a;
	a.add(new MoveItemAction(this, item, this->baseItems.indexOf(item), newIndex));
	return a;
}

template<typename T, typename G>
void Manager<T, G>::reorderItems()
{
	if (comparator.compareFunc != nullptr)
	{
		//baseItems.getLock().enter();
		baseItems.sort(comparator);
		//baseItems.getLock().exit();
		controllableContainers.removeIf([this](ControllableContainer* c) { return baseItems.contains((T*)c); }); //remove if not in items
		controllableContainers.addArray(getItems(false));
	}

	managerListeners.call(&ManagerListener::itemsReordered);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}


template<typename T, typename G>
T* Manager<T, G>::getItemAt(int index) const
{
	if (baseItems.size() == 0) return nullptr;
	if (index < 0 || index >= baseItems.size()) return nullptr;
	if (T* it = dynamic_cast<T*>(baseItems[index])) return it;
	return nullptr;
}

template<typename T, typename G>
T* Manager<T, G>::getFirstItem(bool recursive) const
{
	if (baseItems.size() == 0) return nullptr;

	for (auto& i : baseItems)
	{
		if (G* g = dynamic_cast<G*>(i))
		{
			if (recursive)
			{
				T* result = g->manager.getFirstItem(true);
				if (result != nullptr) return result;
			}
		}
		else if (T* it = dynamic_cast<T*>(i))
		{
			return it;
		}
	}

	return nullptr;
}

template<typename T, typename G>
T* Manager<T, G>::getLastItem(bool recursive) const
{
	if (baseItems.size() == 0) return nullptr;
	for (int i = baseItems.size() - 1; i >= 0; --i)
	{
		if (G* g = dynamic_cast<G*>(baseItems[i]))
		{
			if (recursive)
			{
				T* result = g->manager.getLastItem(true);
				if (result != nullptr) return result;
			}
		}
		else if (T* it = dynamic_cast<T*>(baseItems[i]))
		{
			return it;
		}
	}
	return nullptr;
}



template<typename T, typename G>
T* Manager<T, G>::getItemBefore(T* item) const
{
	if (item == nullptr) return nullptr;
	int index = baseItems.indexOf(item);
	if (index == -1) return nullptr;
	if (index - 1 >= 0)
	{
		if (G* g = dynamic_cast<G*>(baseItems[index - 1]))
		{
			return g->manager.getLastItem(true);
		}
		else
		{
			return dynamic_cast<T*>(baseItems[index - 1]);
		}
	}

	return nullptr;
}

template<typename T, typename G>
T* Manager<T, G>::getItemAfter(T* item) const
{
	if (item == nullptr) return nullptr;
	int index = baseItems.indexOf(item);
	if (index == -1) return nullptr;
	if (index + 1 < baseItems.size())
	{
		if (G* g = dynamic_cast<G*>(baseItems[index + 1]))
		{
			return g->manager.getFirstItem(true);
		}
		else
		{
			return dynamic_cast<T*>(baseItems[index + 1]);
		}
	}

	return nullptr;
}

template<typename T, typename G>
juce::Array<T*> Manager<T, G>::getItems(bool recursive) const
{
	juce::Array<T*> result;
	for (auto& i : baseItems)
	{
		if (G* g = dynamic_cast<G*>(i))
		{
			if (recursive) result.addArray(g->manager.getItems(true));
			continue;
		}
		else
		{
			result.add(static_cast<T*>(i));
		}
	}

	return result;
}

template<typename T, typename G>
juce::Array<G*> Manager<T, G>::getItemGroups(bool recursive) const
{
	juce::Array<ItemGroup<T>*> result;
	for (auto& i : baseItems)
	{
		if (G* g = dynamic_cast<G*>(i))
		{
			result.add(g);
			if (recursive) result.addArray(g->manager.getItemGroups(true));
		}
	}

	return result;
}


template<typename T, typename G>
T* Manager<T, G>::getItemWithName(const juce::String& itemShortName, bool searchItemWithNiceNameToo, bool searchWithLowerCaseIfNotFound, bool recursive)
{
	//const ScopedLock lock(baseItems.getLock());
	for (auto& t : items)
	{
		if (t->shortName == itemShortName) return t;
		if (searchItemWithNiceNameToo && t->niceName == itemShortName) return t;
		if (recursive)
		{
			if (G* g = dynamic_cast<G*>(t))
			{
				if (T* it = g->manager.getItemWithName(itemShortName, searchItemWithNiceNameToo, searchWithLowerCaseIfNotFound, true)) return it;
			}
		}
	}

	if (searchWithLowerCaseIfNotFound)
	{
		for (auto& t : items)
		{
			if (t->shortName.toLowerCase() == itemShortName.toLowerCase()) return t;
			if (searchItemWithNiceNameToo && t->niceName.toLowerCase() == itemShortName.toLowerCase()) return t;
			if (recursive)
			{

				if (G* g = dynamic_cast<G*>(t))
				{
					if (T* it = g->manager.getItemWithName(itemShortName, searchItemWithNiceNameToo, searchWithLowerCaseIfNotFound, true)) return it;
				}
			}
		}
	}

	return nullptr;
}


template<typename T, typename G>
template<typename IType>
juce::Array<IType*> Manager<T, G>::getItemsWithType(bool recursive)
{
	static_assert(std::is_base_of<T, IType>::value, "IType must be derived from T");

	juce::Array<IType*> result;
	for (auto& i : baseItems)
	{
		if (G* g = dynamic_cast<G*>(i))
		{
			if (recursive) result.addArray(g->manager.getItemsWithType<IType>(true));
			continue;
		}

		if (IType* it = dynamic_cast<IType*>(i)) result.add(it);
	}

	return result;
}


template<typename T, typename G>
void Manager<T, G>::callFunctionOnItems(std::function<void(T*)> func, bool recursive)
{
	if (func == nullptr) return;

	for (auto& i : baseItems)
	{
		if (G* g = dynamic_cast<G*>(i))
		{
			if (recursive) g->manager.callFunctionOnItems(func, true);
			continue;
		}

		func(static_cast<T*>(i));
	}
}

template<typename T, typename G>
bool Manager<T, G>::callStoppingFunctionOnItems(std::function<bool(T*)> func, bool recursive)
{
	if (func == nullptr) return false;

	for (auto& i : baseItems)
	{
		if (G* g = dynamic_cast<G*>(i))
		{
			if (recursive) if (!g->manager.callStoppingFunctionOnItems(func, true)) return false;
			continue;
		}

		if (!func(static_cast<T*>(i))) return false;
	}

	return true;
}

template<typename T, typename G>
void Manager<T, G>::handleAddFromRemoteControl(juce::var data)
{
	if (!userCanAddItemsManually) return;
	addItemFromData(data);
}

template<typename T, typename G>
void Manager<T, G>::clear()
{
	isClearing = true;
	//const ScopedLock lock(baseItems.getLock());
	while (baseItems.size() > 0) removeItem(static_cast<T*>(baseItems[0]), false);
	isClearing = false;

#if ORGANICUI_USE_WEBSERVER
	if (notifyRemoteControlOnClear && !isCurrentlyLoadingData && isAttachedToRoot()) OSCRemoteControl::getInstance()->sendPathChangedFeedback(getControlAddress());
#endif
}

template<typename T, typename G>
void Manager<T, G>::askForRemoveBaseItem(BaseItem* item)
{
	removeItem(static_cast<T*>(item));
}

template<typename T, typename G>
void Manager<T, G>::askForDuplicateItem(BaseItem* item)
{
	if (!userCanAddItemsManually) return;
	juce::var data = item->getJSONData();
	data.getDynamicObject()->setProperty("index", baseItems.indexOf(static_cast<T*>(item)) + 1);
	addItemFromData(data);
}

template<typename T, typename G>
void Manager<T, G>::askForPaste()
{
	addItemsFromClipboard();
}

template<typename T, typename G>
void Manager<T, G>::askForMoveBefore(BaseItem* i)
{
	T* item = static_cast<T*>(i);
	setItemIndex(item, juce::jmax(baseItems.indexOf(item) - 1, 0));
	//	int index = baseItems.indexOf(static_cast<T *>(i));
	//	if (index == 0) return;
	//	baseItems.swap(index, index - 1);
	//	controllableContainers.swap(index, index - 1);
	//
	//	managerListeners.call(&ManagerListener::itemsReordered);
	//	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<typename T, typename G>
void Manager<T, G>::askForMoveAfter(BaseItem* i)
{
	T* item = static_cast<T*>(i);
	setItemIndex(item, juce::jmin(baseItems.indexOf(item) + 1, baseItems.size() - 1));
	//int index = baseItems.indexOf(static_cast<T *>(i));
	//if (index == baseItems.size() -1) return;
	//baseItems.swap(index, index + 1);
	//controllableContainers.swap(index, index+1);

	//managerListeners.call(&ManagerListener::itemsReordered);
	//managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<typename T, typename G>
void Manager<T, G>::askForSelectAllItems(bool addToSelection)
{
	int numItems = baseItems.size();
	if (!addToSelection) selectionManager->clearSelection(numItems == 0);
	else deselectThis(numItems == 0);

	if (numItems > 1) for (int i = 0; i < numItems; ++i) getItemAt(i)->selectThis(true, i == numItems - 1); //only notify on last
	else if (numItems > 0) getFirstItem()->selectThis(addToSelection, true);
}

template<typename T, typename G>
void Manager<T, G>::askForSelectPreviousItem(BaseItem* item, bool addToSelection)
{
	int index = baseItems.indexOf(dynamic_cast<T*>(item));
	if (index <= 0) return;
	getItemAt(index - 1)->selectThis(addToSelection);
}

template<typename T, typename G>
void Manager<T, G>::askForSelectNextItem(BaseItem* item, bool addToSelection)
{
	int index = baseItems.indexOf(dynamic_cast<T*>(item));
	if (index == -1 || index >= baseItems.size() - 1) return;
	items[index + 1]->selectThis(addToSelection);
}

template<typename T, typename G>
void Manager<T, G>::onContainerParameterChanged(Parameter* p)
{
	EnablingControllableContainer::onContainerParameterChanged(p);
	if (p == showSnapGrid || p == snapGridSize)
	{
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::NEEDS_UI_UPDATE));
	}
}

template<typename T, typename G>
juce::var Manager<T, G>::getExportSelectionData()
{
	juce::var data;

	for (auto& t : items)
	{
		if (t->isSavable && t->isSelected) data.append(t->getJSONData());
	}

	return data;
}

template<typename T, typename G>
juce::var Manager<T, G>::getJSONData(bool includeNonOverriden)
{
	juce::var data = ControllableContainer::getJSONData(includeNonOverriden);
	juce::var itemsData = juce::var();
	//baseItems.getLock().enter();
	for (auto& t : baseItems)
	{
		if (t->isSavable) itemsData.append(t->getJSONData(includeNonOverriden));
	}
	//baseItems.getLock().exit();

	if (itemsData.size() > 0) data.getDynamicObject()->setProperty("items", itemsData);

	juce::var vData;
	vData.append(viewOffset.x);
	vData.append(viewOffset.y);
	data.getDynamicObject()->setProperty("viewOffset", vData);
	data.getDynamicObject()->setProperty("viewZoom", viewZoom);

	return data;
}

template<typename T, typename G>
void Manager<T, G>::loadJSONDataInternal(juce::var data)
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

template<typename T, typename G>
void Manager<T, G>::loadJSONDataManagerInternal(juce::var data)
{
	juce::var itemsData = data.getProperty("items", juce::var());
	if (itemsData.isVoid()) return;

	addItemsFromData(itemsData, false);
}

template<typename T, typename G>
juce::var Manager<T, G>::getRemoteControlData()
{
	if (isClearing || isCurrentlyLoadingData) return juce::var(new juce::DynamicObject());
	return ControllableContainer::getRemoteControlData();
}

template<typename T, typename G>
void Manager<T, G>::getRemoteControlDataInternal(juce::var& data)
{
	data.getDynamicObject()->setProperty("TYPE", "Manager");

	juce::var extType = juce::var();
	if (managerFactory != nullptr)
	{
		for (auto& d : managerFactory->defs) extType.append(d->menuPath + "/" + d->type);
	}
	else extType.append(itemDataType);

	data.getDynamicObject()->setProperty("EXTENDED_TYPE", extType);
	data.getDynamicObject()->setProperty("BASE_TYPE", itemDataType);
}

template<typename T, typename G>
juce::PopupMenu Manager<T, G>::getItemsMenu(int startID)
{
	juce::PopupMenu menu;
	int numValues = baseItems.size();
	for (int j = 0; j < numValues; j++)
	{
		T* c = items[j];
		menu.addItem(startID + j, c->niceName);
	}
	return menu;
}

template<typename T, typename G>
T* Manager<T, G>::getItemForMenuResultID(int id, int startID)
{
	return items[id - startID];
}

template<typename T, typename G>
T* Manager<T, G>::getFirstSelectedItem()
{
	for (auto& i : baseItems) if (i->isSelected) return i;
	return nullptr;
}

template<typename T, typename G>
juce::String Manager<T, G>::getScriptTargetString()
{
	return "[" + niceName + " : Manager(" + itemDataType + ")]";
}


template<typename T, typename G>
InspectableEditor* Manager<T, G>::getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables)
{
	return new GenericManagerEditor<T>(this, isRoot);

}


// SCRIPT

template<typename T, typename G>
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


template<typename T, typename G>
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

template<typename T, typename G>
juce::var Manager<T, G>::removeAllItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		m->clear();
	}

	return juce::var();
}

template<typename T, typename G>
juce::var Manager<T, G>::getItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	juce::var result = juce::var();
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		for (auto& i : m->items) result.append(i->getScriptObject());
	}

	return result;
}

template<typename T, typename G>
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

template<typename T, typename G>
juce::var Manager<T, G>::getItemAtFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		int index = args.arguments[0];
		if (index < 0 || index >= m->baseItems.size()) return juce::var();
		T* i = m->getItemAt(index);
		if (i != nullptr) return i->getScriptObject();
	}

	return juce::var();
}

template<typename T, typename G>
juce::var Manager<T, G>::getItemIndexFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		if (!args.arguments[0].isObject()) return juce::var();
		T* item = dynamic_cast<T*>((T*)(juce::int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return juce::var();
		return m->baseItems.indexOf(item);
	}

	return juce::var();
}

template<typename T, typename G>
juce::var Manager<T, G>::getItemBeforeFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		if (!args.arguments[0].isObject()) return juce::var();
		T* item = dynamic_cast<T*>((T*)(juce::int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return juce::var();
		int index = m->baseItems.indexOf(item);
		if (index <= 0) return juce::var();
		return m->getItemAt(index - 1)->getScriptObject();
	}

	return juce::var();
}

template<typename T, typename G>
juce::var Manager<T, G>::getItemAfterFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		if (!checkNumArgs(m->niceName, args, 1)) return juce::var();
		if (!args.arguments[0].isObject()) return juce::var();
		T* item = dynamic_cast<T*>((T*)(juce::int64)args.arguments[0].getDynamicObject()->getProperty(scriptPtrIdentifier));
		if (item == nullptr) return juce::var();
		int index = m->baseItems.indexOf(item);
		if (index >= m->baseItems.size() - 1) return juce::var();
		return m->items[index + 1]->getScriptObject();
	}

	return juce::var();
}

template<typename T, typename G>
juce::var Manager<T, G>::reorderItemsFromScript(const juce::var::NativeFunctionArgs& args)
{
	if (Manager<T, G>* m = getObjectFromJS<Manager<T, G>>(args))
	{
		m->reorderItems();
	}

	return juce::var();
}


//MANAGER EVENT

template<typename T, typename G>
Manager<T, G>::ManagerEvent::ManagerEvent(Type t, T* i) : type(t)
{
	itemsRef.add(i);
}

template<typename T, typename G>
Manager<T, G>::ManagerEvent::ManagerEvent(Type t, juce::Array<T*> iList) : type(t)
{
	for (auto& i : iList)
	{
		itemsRef.add(dynamic_cast<Inspectable*>(i));
	}
}

template<typename T, typename G>
juce::Array<T*> Manager<T, G>::ManagerEvent::getItems() const
{
	juce::Array<T*> result;
	for (auto& i : itemsRef)
	{
		if (i != nullptr && !i.wasObjectDeleted()) result.add(dynamic_cast<T*>(i.get()));
	}
	return result;
}

template<typename T, typename G>
T* Manager<T, G>::ManagerEvent::getItem(int index) const
{
	if (itemsRef.size() > index && itemsRef[index] != nullptr && !itemsRef[index].wasObjectDeleted()) return static_cast<T*>(itemsRef[index].get());
	return nullptr;
}







//ACTIONS

template<typename T, typename G>
Manager<T, G>::ManagerBaseAction::ManagerBaseAction(Manager* manager, juce::var _data) :
	managerControlAddress(manager->getControlAddress()),
	data(_data),
	managerRef(manager)
{
}

template<typename T, typename G>
Manager<T, G>* Manager<T, G>::ManagerBaseAction::getManager() {
	if (managerRef != nullptr && !managerRef.wasObjectDeleted()) return dynamic_cast<Manager<T, G>*>(managerRef.get());
	else if (Engine::mainEngine != nullptr)
	{
		ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(managerControlAddress, true);
		if (cc != nullptr) return dynamic_cast<Manager<T, G>*>(cc);
	}

	return nullptr;
}

template<typename T, typename G>
Manager<T, G>::ItemBaseAction::ItemBaseAction(Manager* m, T* i, juce::var data) :
	ManagerBaseAction(m, data),
	itemRef(i),
	itemIndex(0)
{
	T* s = getItem();
	if (s != nullptr)
	{
		this->itemShortName = dynamic_cast<BaseItem*>(s)->shortName;
		this->itemIndex = data.getProperty("index", m->baseItems.indexOf(s));

	}
}

template<typename T, typename G>
T* Manager<T, G>::ItemBaseAction::getItem()
{
	if (itemRef != nullptr && !itemRef.wasObjectDeleted()) return dynamic_cast<T*>(itemRef.get());
	else
	{
		Manager* m = this->getManager();
		if (m != nullptr) return m->getItemWithName(itemShortName);
	}

	return nullptr;
}

template<typename T, typename G>
Manager<T, G>::AddItemAction::AddItemAction(Manager* m, T* i, juce::var data) : ItemBaseAction(m, i, data) {
}

template<typename T, typename G>
bool Manager<T, G>::AddItemAction::perform()
{
	Manager* m = this->getManager();
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

template<typename T, typename G>
bool Manager<T, G>::AddItemAction::undo()
{
	T* s = this->getItem();
	if (s == nullptr) return false;
	this->data = s->getJSONData();
	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(s, false);
	this->itemRef = nullptr;
	return true;
}

template<typename T, typename G>
Manager<T, G>::RemoveItemAction::RemoveItemAction(Manager* m, T* i, juce::var data) : ItemBaseAction(m, i, data)
{

}

template<typename T, typename G>
bool Manager<T, G>::RemoveItemAction::perform()
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

template<typename T, typename G>
bool Manager<T, G>::RemoveItemAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;
	this->itemRef = m->addItemFromData(this->data, false);
	return true;
}


template<typename T, typename G>
Manager<T, G>::MoveItemAction::MoveItemAction(Manager* m, T* i, int prevIndex, int newIndex) :
	ItemBaseAction(m, i),
	prevIndex(prevIndex),
	newIndex(newIndex)
{

}

template<typename T, typename G>
bool Manager<T, G>::MoveItemAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	T* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, newIndex, false);
	return true;
}

template<typename T, typename G>
bool Manager<T, G>::MoveItemAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	T* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, prevIndex, false);
	return true;
}


template<typename T, typename G>
Manager<T, G>::ItemsBaseAction::ItemsBaseAction(Manager* m, juce::Array<T*> iList, juce::var data) :
	ManagerBaseAction(m, data)
{
	if (data.isVoid())
	{
		std::sort(iList.begin(), iList.end(),
			[m](const auto& lhs, const auto& rhs) { return m->baseItems.indexOf(lhs) < m->baseItems.indexOf(rhs); });
	}

	for (auto& i : iList)
	{
		BaseItem* bi = dynamic_cast<BaseItem*>(i);
		itemsRef.add(bi);
		itemsShortName.add(bi != nullptr ? bi->shortName : "");
	}
}

template<typename T, typename G>
juce::Array<T*> Manager<T, G>::ItemsBaseAction::getItems()
{
	juce::Array<T*> iList;
	int index = 0;
	for (auto& i : itemsRef)
	{
		if (i != nullptr && !i.wasObjectDeleted())
		{
			T* ti = dynamic_cast<T*>(i.get());
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

template<typename T, typename G>
Manager<T, G>::AddItemsAction::AddItemsAction(Manager* m, juce::Array<T*> iList, juce::var data) : ItemsBaseAction(m, iList, data)
{
}

template<typename T, typename G>
bool Manager<T, G>::AddItemsAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	juce::Array<T*> iList = this->getItems();
	if (!iList.isEmpty()) m->addItems(iList, this->data, false);
	else if (!this->data.isVoid())
	{
		juce::Array<T*> newList = m->addItemsFromData(this->data, false);
		this->itemsRef.clear();
		for (auto& i : newList) this->itemsRef.add(i);
	}


	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}

template<typename T, typename G>
bool Manager<T, G>::AddItemsAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr)
	{
		this->itemsRef.clear();
		return false;
	}

	juce::Array<T*> iList = this->getItems();
	this->data = juce::var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			juce::var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->baseItems.indexOf(i));
			this->data.append(d);
		}
	}
	m->removeItems(iList, false);
	return true;
}

template<typename T, typename G>
Manager<T, G>::RemoveItemsAction::RemoveItemsAction(Manager* m, juce::Array<T*> iList) : ItemsBaseAction(m, iList) {
}

template<typename T, typename G>
bool Manager<T, G>::RemoveItemsAction::perform()
{
	Manager* m = this->getManager();
	if (m == nullptr)
	{
		this->itemsRef.clear();
		return false;
	}

	juce::Array<T*> iList = this->getItems();
	this->data = juce::var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			juce::var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->baseItems.indexOf(i));
			this->data.append(d);
		}
	}

	m->removeItems(iList, false);
	return true;
}

template<typename T, typename G>
bool Manager<T, G>::RemoveItemsAction::undo()
{
	Manager* m = this->getManager();
	if (m == nullptr) return false;

	juce::Array<T*> iList = m->addItemsFromData(this->data, false);

	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}