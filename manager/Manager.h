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
	public BaseManager
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	static_assert(std::is_base_of<ItemGroup<T>, G>::value, "G must be derived from ItemGroup<T>");
	static_assert(std::is_base_of<BaseItem, G>::value, "G must be derived from BaseItem");

	Manager(const juce::String& name = "Items", bool canHaveGroups = false);
	virtual ~Manager() {}


	juce::Array<T*, juce::CriticalSection> items;
	juce::Array<G*, juce::CriticalSection> groups;

	//Factory
	Factory<T, G>* managerFactory;

	void setHasGridOptions(bool hasGridOptions);


	virtual T* createItem(); //to override if special constructor to use
	virtual G* createGroup(); //to override if special constructor to use

	virtual BaseItem* createItemFromData(juce::var data) override;

	T* addItem(T* item = nullptr, juce::var data = juce::var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data
	G* addGroup(G* item = nullptr, juce::var data = juce::var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data

	juce::Array<T*> addItems(juce::Array<T*> items, juce::var data = juce::var(), bool addToUndo = true, bool notify = true);
	void addItemManagerInternal(BaseItem* item, juce::var data = juce::var()) override;
	void addItemsManagerInternal(juce::Array<BaseItem*> item, juce::var data = juce::var()) override;
	//virtual T* addItemFromData(juce::var data, bool addToUndo = true);
	//virtual juce::Array<T*> addItemsFromData(juce::var data, bool addToUndo = true);
	//virtual juce::Array<T*> addItemsFromClipboard(bool showWarning = true);

	bool canAddItemOfType(const juce::String& typeToCheck) override;

	void removeItems(juce::Array<T*> itemsToRemove, bool addToUndo = true, bool notify = true);

	void removeItemManagerInternal(BaseItem* item) override;
	void removeItemsManagerInternal(juce::Array<BaseItem*> item) override;

	virtual juce::UndoableAction* getAddItemUndoableAction(T* item = nullptr, juce::var data = juce::var());
	virtual juce::UndoableAction* getAddItemsUndoableAction(juce::Array<T*> item = nullptr, juce::var data = juce::var());
	virtual juce::Array<juce::UndoableAction*> getRemoveItemUndoableAction(T* item);
	virtual juce::Array<juce::UndoableAction*> getRemoveItemsUndoableAction(juce::Array<T*> items);

	//to override for specific handling like adding custom listeners, etc.
	virtual void addItemInternal(T*, juce::var data) {}
	virtual void addItemsInternal(juce::Array<T*>, juce::var data) {}
	virtual void removeItemInternal(T*) {}
	virtual void removeItemsInternal(juce::Array<T*>) {}

	virtual void addGroupInternal(G*, juce::var data) {}
	virtual void addGroupsInternal(juce::Array<G*>, juce::var data) {}
	virtual void removeGroupInternal(G*) {}
	virtual void removeGroupsInternal(juce::Array<G*>) {}

	virtual void reorderItems() override; //to be overriden if needed

	void notifyItemAdded(BaseItem* item, bool fromChildGroup = false) override;
	void notifyItemsAdded(juce::Array<BaseItem*> items, bool fromChildGroup = false) override;
	void notifyItemRemoved(BaseItem* item, bool fromChildGroup = false) override;
	void notifyItemsRemoved(juce::Array<BaseItem*> items, bool fromChildGroup = false) override;
	void notifyItemsReordered(bool fromChildGroup = false) override;

	virtual juce::Array<T*> getItems(bool includeDisabled = true, bool recursive = true);


	virtual T* getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo = false, bool searchWithLowerCaseIfNotFound = true) override;

	virtual T* getItemWithPath(const juce::String& relativePath);

	juce::PopupMenu getItemsMenu(int startID);
	T* getItemForMenuResultID(int id, int startID);

	T* getFirstSelectedItem();

	virtual void getRemoteControlDataInternal(juce::var& data) override;

	void notifyAsync(BaseManagerEvent::Type type, juce::Array<BaseItem*> _items = juce::Array<BaseItem*>(), bool fromChildGroup = false) override;

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

	class ManagerEvent :
		public BaseManagerEvent
	{
	public:
		ManagerEvent(BaseManagerEvent::Type type, juce::Array<BaseItem*> items = juce::Array<BaseItem*>(), bool fromChildGroup = false) :
			BaseManagerEvent(type, items, fromChildGroup)
		{
		}

		~ManagerEvent() {}

		bool isGroup() const {
			if (baseItems.isEmpty()) return false;
			if (baseItems.getFirst().wasObjectDeleted()) return false;
			return baseItems.getFirst()->isGroup;
		}

		juce::Array<T*> getItems() const;
		T* getItem(int index = 0) const;

		juce::Array<G*> getGroups() const;
		G* getGroup(int index = 0) const;

		juce::Array<BaseItem*> getBaseItems() const
		{
			juce::Array<BaseItem*> items;
			for (auto& i : baseItems)
			{
				if (i == nullptr) continue;
				if (i.wasObjectDeleted()) continue;
				items.add(i);
			}
			return items;
		}
	};

	using BManagerEvent = typename Manager<T, G>::ManagerEvent;
	using ManagerNotifier = QueuedNotifier<BManagerEvent>;
	ManagerNotifier managerNotifier;
	ManagerNotifier recursiveManagerNotifier;
	typedef typename QueuedNotifier<BManagerEvent>::Listener AsyncListener;

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

	juce::Array<T*> getArrayAsItems(juce::Array<BaseItem*> baseItems) const
	{
		juce::Array<T*> result;
		for (auto& i : baseItems) if (T* it = dynamic_cast<T*>(i)) result.add(it);
		return result;
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Manager);


};


template<class T, class G>
Manager<T, G>::Manager(const juce::String& name, bool canHaveGroups) :
	BaseManager(name, canHaveGroups),
	managerFactory(nullptr),
	managerNotifier(50),
	recursiveManagerNotifier(50),
	comparator(this)
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
G* Manager<T, G>::createGroup()
{
	return new G(new Manager<T, G>());
}

template<class T, class G>
T* Manager<T, G>::addItem(T* item, juce::var data, bool addToUndo, bool notify)
{

	jassert(items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	if (item == nullptr) return nullptr; //could not create here
	BaseManager::addItem(item, data, addToUndo, notify);
	return item;
}

template<class T, class G>
G* Manager<T, G>::addGroup(G* group, juce::var data, bool addToUndo, bool notify)
{

	jassert(groups.indexOf(group) == -1); //be sure item is no here already
	if (group == nullptr) group = createGroup();
	if (group == nullptr) return nullptr; //could not create here
	BaseManager::addItem(group, data, addToUndo, notify);
	return group;
}

template<class T, class G>
juce::Array<T*> Manager<T, G>::addItems(juce::Array<T*> items, juce::var data, bool addToUndo, bool notify)
{
	juce::Array<BaseItem*> itemsToAdd;
	for (auto& i : items) if (BaseItem* it = dynamic_cast<BaseItem*>(i)) itemsToAdd.add(it);
	juce::Array<BaseItem*> newItems = BaseManager::addItems(itemsToAdd, data, addToUndo, notify);
	return getArrayAsItems(newItems);
}

template<class T, class G>
void Manager<T, G>::addItemManagerInternal(BaseItem* item, juce::var data)
{
	if (!item->isGroup && (!canHaveGroups || item == baseItems.getLast()))
	{
		this->items.insert(baseItems.indexOf(item), (T*)item);
	}
	else
	{
		//recreate the items array because it may not be in the same order as baseItems
		if (item->isGroup)
		{
			groups.clear();
			for (auto& i : baseItems) if (G* it = dynamic_cast<G*>(i)) groups.add(it);
		}
		else
		{
			items.clear();
			for (auto& i : baseItems) if (T* it = dynamic_cast<T*>(i)) items.add(it);
		}
	}

	if (item->isGroup) addGroupInternal((G*)item, data);
	else addItemInternal((T*)item, data);
}

template<class T, class G>
void Manager<T, G>::addItemsManagerInternal(juce::Array<BaseItem*> item, juce::var data)
{
	juce::Array<T*> itemsToAdd;
	for (auto& i : item) if (T* it = dynamic_cast<T*>(i)) itemsToAdd.add(it);
	addItemsInternal(itemsToAdd, data);
}

//template<class T, class G>
//T* Manager<T, G>::addItemFromData(juce::var data, bool addToUndo)
//{
//	return (T*)BaseManager::addItemFromData(data, addToUndo);
//}
//
//template<class T, class G>
//juce::Array<T*> Manager<T, G>::addItemsFromData(juce::var data, bool addToUndo)
//{
//	juce::Array<BaseItem*> itemsAdded = BaseManager::addBaseItemsFromData(data, addToUndo);
//	return getArrayAsItems(itemsAdded);
//}
//
//template<class T, class G>
//juce::Array<T*> Manager<T, G>::addItemsFromClipboard(bool showWarning)
//{
//	juce::Array<BaseItem*> itemsAdded = BaseManager::addBaseItemsFromClipboard(showWarning);
//	return getArrayAsItems(itemsAdded);
//}

template<class T, class G>
bool Manager<T, G>::canAddItemOfType(const juce::String& typeToCheck)
{
	if (this->managerFactory != nullptr && this->managerFactory->hasDefinitionWithType(typeToCheck)) return true;
	return BaseManager::canAddItemOfType(typeToCheck);
}

template<class T, class G>
void Manager<T, G>::removeItems(juce::Array<T*> _items, bool addToUndo, bool notify)
{
	juce::Array<BaseItem*> itemsToRemove;
	for (auto& i : _items) itemsToRemove.add(i);
	BaseManager::removeItems(itemsToRemove, addToUndo, notify);
}

template<class T, class G>
void Manager<T, G>::removeItemManagerInternal(BaseItem* item)
{
	if (item->isGroup)
	{
		groups.removeAllInstancesOf((G*)item);
		removeGroupInternal((G*)item);
	}
	else
	{
		items.removeAllInstancesOf((T*)item);
		removeItemInternal((T*)item);
	}
}

template<class T, class G>
void Manager<T, G>::removeItemsManagerInternal(juce::Array<BaseItem*> item)
{
	juce::Array<T*> itemsToRemove;
	for (auto& i : item) if (T* it = dynamic_cast<T*>(i)) itemsToRemove.add(it);
	removeItemsInternal(itemsToRemove);
}


template<class T, class G>
juce::UndoableAction* Manager<T, G>::getAddItemUndoableAction(T* item, juce::var data)
{
	return BaseManager::getAddBaseItemUndoableAction(item, data);
}

template<class T, class G>
juce::UndoableAction* Manager<T, G>::getAddItemsUndoableAction(juce::Array<T*> _items, juce::var data)
{
	juce::Array<BaseItem*> itemsToAdd;
	for (auto& i : _items) if (BaseItem* it = dynamic_cast<BaseItem*>(i)) itemsToAdd.add(it);
	return BaseManager::getAddBaseItemsUndoableAction(itemsToAdd, data);
}

template<class T, class G>
juce::Array<juce::UndoableAction*> Manager<T, G>::getRemoveItemUndoableAction(T* item)
{
	return BaseManager::getRemoveBaseItemUndoableAction(item);
}

template<class T, class G>
juce::Array<juce::UndoableAction*> Manager<T, G>::getRemoveItemsUndoableAction(juce::Array<T*> _items)
{
	juce::Array<BaseItem*> itemsToRemove;
	for (auto& i : _items) if (BaseItem* it = dynamic_cast<BaseItem*>(i)) itemsToRemove.add(it);
	return BaseManager::getRemoveBaseItemsUndoableAction(itemsToRemove);
}

template<class T, class G>
void Manager<T, G>::reorderItems()
{
	if (comparator.compareFunc != nullptr)
	{
		//items.getLock().enter();
		items.sort(comparator);
		//items.getLock().exit();
		controllableContainers.removeIf([this](ControllableContainer* c) { return items.contains((T*)c); }); //remove if not in items
		controllableContainers.addArray(items);
	}

	notifyItemsReordered();
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
	notifyAsync(ManagerEvent::ITEMS_REORDERED, Array<BaseItem*>(), fromChildGroup);
}

template<class T, class G>
juce::Array<T*> Manager<T, G>::getItems(bool includeDisabled, bool recursive)
{
	juce::Array<BaseItem*> baseItems = BaseManager::getBaseItems(recursive, includeDisabled, false);
	juce::Array<T*> result;
	for (auto& i : baseItems) if (T* it = dynamic_cast<T*>(i)) result.add(it);
	return result;
}

template<class T, class G>
T* Manager<T, G>::getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo, bool searchWithLowerCaseIfNotFound)
{
	return (T*)BaseManager::getItemWithName(itemShortName, searchNiceNameToo, searchWithLowerCaseIfNotFound);
}

template<class T, class G>
T* Manager<T, G>::getItemWithPath(const juce::String& relativePath)
{
	return (T*)BaseManager::getItemWithPath(relativePath);
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
void Manager<T, G>::getRemoteControlDataInternal(juce::var& data)
{
	juce::var extType = juce::var();
	if (managerFactory != nullptr)
	{
		for (auto& d : managerFactory->defs) extType.append(d->menuPath + "/" + d->type);
	}
	else extType.append(itemDataType);

	data.getDynamicObject()->setProperty("EXTENDED_TYPE", extType);
}

template<class T, class G>
void Manager<T, G>::notifyAsync(BaseManagerEvent::Type type, juce::Array<BaseItem*> _items, bool fromChildGroup)
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
	for (auto& i : baseItems)
	{
		if (i != nullptr && !i.wasObjectDeleted() && !i->isGroup) result.add(dynamic_cast<T*>(i.get()));
	}
	return result;
}

template<class T, class G>
T* Manager<T, G>::ManagerEvent::getItem(int index) const
{
	if (baseItems.size() > index && baseItems[index] != nullptr && !baseItems[index].wasObjectDeleted()) return static_cast<T*>(baseItems[index].get());
	return nullptr;
}

template<class T, class G>
juce::Array<G*> Manager<T, G>::ManagerEvent::getGroups() const
{
	juce::Array<G*> result;
	for (auto& i : baseItems)
	{
		if (i != nullptr && !i.wasObjectDeleted()) result.add(dynamic_cast<G*>(i.get()));
	}
	return result;
}

template<class T, class G>
G* Manager<T, G>::ManagerEvent::getGroup(int index) const
{
	if (baseItems.size() > index && baseItems[index] != nullptr && !baseItems[index].wasObjectDeleted()) return static_cast<G*>(baseItems[index].get());
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