/*
  ==============================================================================

	BaseManager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#ifndef BASEMANAGER_H_INCLUDED
#define BASEMANAGER_H_INCLUDED




template <class T>
class BaseManager :
	public EnablingControllableContainer,
	public BaseItem::Listener
{
public:
	BaseManager<T>(const String &name);
	virtual ~BaseManager<T>();

	OwnedArray<T> items;

	//Factory
	Factory<T> * managerFactory;
	String itemDataType;


	bool userCanAddItemsManually;
	bool selectItemWhenCreated;
	bool autoReorderOnAdd;

	virtual T * createItem(); //to override if special constructor to use
	virtual T * addItemFromData(var data, bool addToUndo = true); //to be overriden for specific item creation (from data)
	virtual Array<T *> addItemsFromData(var data, bool addToUndo = true); //to be overriden for specific item creation (from data)
	virtual T * addItemFromClipboard(bool showWarning = true);
	virtual bool canAddItemOfType(const String &typeToCheck);

	virtual void loadItemsData(var data);


	virtual UndoableAction * getAddItemUndoableAction(T * item = nullptr, var data = var());

	T * addItem(T * item = nullptr, var data = var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data
	T * addItem(const Point<float> initialPosition, bool addToUndo = true, bool notify = true);
	Array<T *> addItems(Array<T *> items, var data = var(), bool addToUndo = true);


	virtual Array<UndoableAction *> getRemoveItemUndoableAction(T * item);
	virtual Array<UndoableAction *> getRemoveItemsUndoableAction(Array<T *> items);

	void removeItems(Array<T *> items, bool addToUndo = true);
	void removeItem(T * item, bool addToUndo = true, bool notify = true);

	virtual void reorderItems(); //to be overriden if needed

	//to override for specific handling like adding custom listeners, etc.
	virtual void addItemInternal(T *, var data) {}
	virtual void removeItemInternal(T *) {}


	T * getItemWithName(const String &itemShortName, bool searchNiceNameToo = false);

    virtual void clear() override;
	void askForRemoveBaseItem(BaseItem * item) override;
	void askForDuplicateItem(BaseItem * item) override;
	void askForPaste() override;
	void askForMoveBefore(BaseItem *) override;
	void askForMoveAfter(BaseItem *) override;

	virtual var getJSONData() override;
	virtual void loadJSONDataInternal(var data) override;
	virtual void loadJSONDataManagerInternal(var data);

	void updateLiveScriptObjectInternal(DynamicObject * = nullptr) override;

	PopupMenu getItemsMenu(int startID);
	T * getItemForMenuResultID(int id, int startID);

	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void itemAdded(T *) {}
		virtual void itemsAdded(Array<T *>) {}
		virtual void itemRemoved(T *) {}
		virtual void itemsRemoved(Array<T *>) {}
		virtual void itemsReordered() {}
	};

	ListenerList<Listener> baseManagerListeners;
	void addBaseManagerListener(Listener* newListener) { baseManagerListeners.add(newListener); }
	void removeBaseManagerListener(Listener* listener) { baseManagerListeners.remove(listener); }

	class ManagerEvent
	{
	public:
		enum Type { ITEM_ADDED, ITEM_REMOVED, ITEMS_REORDERED, ITEMS_ADDED, ITEMS_REMOVED, MANAGER_CLEARED };

		ManagerEvent(Type t, T * i = nullptr) : type(t)
		{
			itemsRef.add(i);
		}

		ManagerEvent(Type t, Array<T *> iList) : type(t)
		{
			for (auto &i : iList)
			{
				itemsRef.add(dynamic_cast<Inspectable *>(i));
			}
		}

		Type type;
		Array<WeakReference<Inspectable>> itemsRef;

		Array<T *> getItems() const
		{
			Array<T *> result;
			for (auto &i : itemsRef)
			{
				if (i != nullptr && !i.wasObjectDeleted()) result.add(dynamic_cast<T *>(i.get()));
			}
			return result;
		}

		T * getItem(int index = 0) const
		{
			if (itemsRef.size() > index && itemsRef[index] != nullptr && !itemsRef[index].wasObjectDeleted()) return static_cast<T *>(itemsRef[index].get());
			return nullptr;
		}
	};

	using BManagerEvent = typename BaseManager<T>::ManagerEvent;
	using ManagerNotifier = QueuedNotifier<BManagerEvent>;
	ManagerNotifier managerNotifier;
	typedef typename QueuedNotifier<BManagerEvent>::Listener AsyncListener;

	void addAsyncManagerListener(AsyncListener * newListener) { managerNotifier.addListener(newListener); }
	void addAsyncCoalescedManagerListener(AsyncListener * newListener) { managerNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncManagerListener(AsyncListener * listener) { managerNotifier.removeListener(listener); }


	InspectableEditor * getEditor(bool /*isRoot*/) override;

	//UNDO MANAGER
	class ManagerBaseAction :
		public UndoableAction
	{
	public:
		ManagerBaseAction(BaseManager * manager, var _data = var()) :
			managerControlAddress(manager->getControlAddress()),
			data(_data),
			managerRef(manager)
		{}

		String managerControlAddress;
		var data;
		WeakReference<Inspectable> managerRef;

		BaseManager<T> * getManager() {
			if (managerRef != nullptr && !managerRef.wasObjectDeleted()) return dynamic_cast<BaseManager<T> *>(managerRef.get());
			else if(Engine::mainEngine != nullptr)
			{
				ControllableContainer * cc = Engine::mainEngine->getControllableContainerForAddress(managerControlAddress, true);
				if (cc != nullptr) return dynamic_cast<BaseManager<T> *>(cc);
			}

			return nullptr;
		}
	};

	class ItemBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemBaseAction(BaseManager * m, T * i, var data = var()) :
			ManagerBaseAction(m, data),
			itemRef(i)
		{
			T * s = getItem();
			if (s != nullptr)
			{
				this->itemShortName = dynamic_cast<BaseItem *>(s)->shortName;
				this->itemIndex = m->items.indexOf(s);

			}
		}

		WeakReference<Inspectable> itemRef;
		String itemShortName;
		int itemIndex;

		T * getItem()
		{
			if (itemRef != nullptr && !itemRef.wasObjectDeleted()) return dynamic_cast<T *>(itemRef.get());
			else
			{
				BaseManager * m = this->getManager();
				if (m != nullptr) return m->getItemWithName(itemShortName);
			}

			return nullptr;
		}
	};

	class AddItemAction :
		public ItemBaseAction
	{
	public:
		AddItemAction(BaseManager * m, T * i, var data = var()) : ItemBaseAction(m, i, data) {
		}

		bool perform() override
		{
			BaseManager * m = this->getManager();
			if (m == nullptr)
			{
				return false;
			}

			T * item = this->getItem();
			if (item != nullptr)
			{
				m->addItem(item, this->data, false);
			} else
			{
				item = m->addItemFromData(this->data, false);
			}

			if (item == nullptr) return false;

			this->itemShortName = item->shortName;
			return true;
		}

		bool undo() override
		{
			T * s = this->getItem();
			if (s == nullptr) return false;
			this->data = s->getJSONData();
			this->data.getDynamicObject()->setProperty("index", this->itemIndex);

			this->getManager()->removeItem(s, false);
			this->itemRef = nullptr;
			return true;
		}
	};

	class RemoveItemAction :
		public ItemBaseAction
	{
	public:
		RemoveItemAction(BaseManager * m, T * i, var data = var()) : ItemBaseAction(m, i, data) {
		}

		bool perform() override
		{

			T * s = this->getItem();

			if (s == nullptr) return false;

			this->data = s->getJSONData();
			this->data.getDynamicObject()->setProperty("index", this->itemIndex);

			this->getManager()->removeItem(s, false);
			this->itemRef = nullptr;
			return true;
		}

		bool undo() override
		{
			BaseManager * m = this->getManager();
			if (m == nullptr) return false;
			this->itemRef = m->addItemFromData(this->data, false);
			return true;
		}
	};

	//Multi add/remove items actions
	class ItemsBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemsBaseAction(BaseManager * m, Array<T *> iList, var data = var()) :
			ManagerBaseAction(m, data)
		{

			for (auto &i : iList)
			{
				BaseItem * bi = dynamic_cast<BaseItem *>(i);
				itemsRef.add(bi);
				itemsShortName.add(bi != nullptr ? bi->shortName : "");
			}
		}

		Array<WeakReference<Inspectable>> itemsRef;
		StringArray itemsShortName;

		Array<T *> getItems()
		{
			Array<T *> iList;
			int index = 0;
			for (auto &i : itemsRef)
			{
				if (i != nullptr && !i.wasObjectDeleted())
				{
					T * ti = dynamic_cast<T *>(i.get());
					if (ti != nullptr) iList.add(ti);
					else
					{
						BaseManager * m = this->getManager();
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
	};

	class AddItemsAction :
		public ItemsBaseAction
	{
	public:
		AddItemsAction(BaseManager * m, Array<T *> iList, var data = var()) : ItemsBaseAction(m, iList, data) {
		}

		bool perform() override
		{
			BaseManager * m = this->getManager();
			if (m == nullptr) return false;

			Array<T *> iList = this->getItems();
			m->addItems(iList, this->data, false);

			this->itemsShortName.clear();
			for (auto &i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
			return true;
		}

		bool undo() override
		{
			Array<T *> iList = this->getItems();
			this->data = var();
			for (auto & i : iList) if (i != nullptr) this->data.append(i->getJSONData());
			BaseManager * m = this->getManager();
			if (m != nullptr) m->removeItems(iList, false);
			this->itemsRef.clear();
			return true;
		}
	};


	class RemoveItemsAction :
		public ItemsBaseAction
	{
	public:
		RemoveItemsAction(BaseManager * m, Array<T *> iList) : ItemsBaseAction(m, iList) {
		}

		bool perform() override
		{
			Array<T *> iList = this->getItems();
			this->data = var();
			for (auto & i : iList) if (i != nullptr) this->data.append(i->getJSONData());
			BaseManager * m = this->getManager();
			if (m != nullptr) m->removeItems(iList, false);
			this->itemsRef.clear();
			return true;
		}

		bool undo() override
		{
			BaseManager * m = this->getManager();
			if (m == nullptr) return false;

			Array<T *> iList = m->addItemsFromData(this->data, false);

			this->itemsShortName.clear();
			for (auto &i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
			return true;
		}
	};


	class ManagerItemComparator
	{
	public:
		ManagerItemComparator(BaseManager * manager) : m(manager), compareFunc(nullptr)
		{
			compareFunc = nullptr;
		}

		BaseManager * m;

		std::function<int(T *, T*)> compareFunc;
		int compareElements(ControllableContainer * i1, ControllableContainer * i2)
		{
			jassert(compareFunc != nullptr);
			return compareFunc(static_cast<T *>(i1), static_cast<T *>(i2));
		}

	};
	
	ManagerItemComparator comparator;


	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseManager<T>)
};


template<class T>
BaseManager<T>::BaseManager(const String & name) :
	EnablingControllableContainer(name, false),
	managerFactory(nullptr),
	itemDataType(""),
	userCanAddItemsManually(true),
	selectItemWhenCreated(true),
	autoReorderOnAdd(true),
	managerNotifier(50),
    comparator(this)
{

	//setCanHavePresets(false);
	//hideInEditor = true;

	skipLabelInTarget = true; //by default manager label in targetParameter UI are not interesting
	nameCanBeChangedByUser = false;
}

template<class T>
BaseManager<T>::~BaseManager()
{

	clear();
}

template<class T>
T * BaseManager<T>::createItem() {
	return new T();
}


template<class T>
UndoableAction * BaseManager<T>::getAddItemUndoableAction(T * item, var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	jassert(items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	return new AddItemAction(this, item, data);
}

template<class T>
T * BaseManager<T>::addItem(T * item, var data, bool addToUndo, bool notify)
{

	jassert(items.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	BaseItem * bi = static_cast<BaseItem *>(item);

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performAction("Add " + bi->niceName, new AddItemAction(this, item, data));
			return item;
		}
	}

	int targetIndex = data.getProperty("index", -1);
	if(targetIndex != -1) items.insert(targetIndex, item);
	else
	{
		if (autoReorderOnAdd && !isCurrentlyLoadingData && comparator.compareFunc != nullptr) items.addSorted(comparator, item);
		else items.add(item);
	}

	bi->addBaseItemListener(this);

	if (!data.isVoid())
	{
		bi->loadJSONData(data);
	}

	//bi->setNiceName(bi->niceName); //force setting a unique name if already taken, after load data so if name is the same as another, will change here

	addChildControllableContainer(bi, items.indexOf(item), notify);

	//if(autoReorderOnAdd) reorderItems();

	addItemInternal(item, data);

	if (notify)
	{
		baseManagerListeners.call(&BaseManager::Listener::itemAdded, item);
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_ADDED, item));

	}

	if (selectItemWhenCreated) bi->selectThis();

	return item;
}

template<class T>
T * BaseManager<T>::addItem(const Point<float> initialPosition, bool addToUndo, bool notify)
{
	T * i = createItem();
	i->viewUIPosition->setPoint(initialPosition);
	addItem(i, addToUndo, notify);
	return i;
}

template<class T>
Array<T *> BaseManager<T>::addItems(Array<T *> itemsToAdd, var data, bool addToUndo)
{

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		AddItemsAction * a = new AddItemsAction(this, itemsToAdd, data);
		UndoMaster::getInstance()->performAction("Add " + String(itemsToAdd.size()) + " items", a);
		return itemsToAdd;
	}

	for (int i = 0; i < itemsToAdd.size(); i++)
	{
		addItem(itemsToAdd[i], data.isArray() ? data[i] : var(), false, false);
	}

	notifyStructureChanged();

	baseManagerListeners.call(&Listener::itemsAdded, itemsToAdd);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_ADDED, itemsToAdd));

	reorderItems();

	return itemsToAdd;
}

//if data is not empty, load data
template<class T>
T * BaseManager<T>::addItemFromData(var data, bool addToUndo)
{
	if (managerFactory != nullptr)
	{
		String type = data.getProperty("type", "");
		if (type.isEmpty()) return nullptr;
		T * i = managerFactory->create(type);
		if (i != nullptr) return addItem(i, data, addToUndo);
	} else
	{
		return addItem(createItem(), data, addToUndo);
	}

	return nullptr;
}

template<class T>
Array<T*> BaseManager<T>::addItemsFromData(var data, bool addToUndo)
{
	Array<T *> itemsToAdd;
	if (managerFactory != nullptr)
	{
		for (int i = 0; i < data.size(); i++)
		{
			String type = data[i].getProperty("type", "");
			if (type.isEmpty()) return nullptr;
			T * it = managerFactory->create(type);
			if(it != nullptr) itemsToAdd.add(it);
		}
	} else
	{
		for (int i = 0; i < data.size(); i++)  itemsToAdd.add(createItem());
	}

	return addItems(itemsToAdd, data, addToUndo);
}

template<class T>
T * BaseManager<T>::addItemFromClipboard(bool showWarning)
{
	if (!userCanAddItemsManually) return nullptr;
	String s = SystemClipboard::getTextFromClipboard();
	var data = JSON::parse(s);
	if (data.isVoid()) return nullptr;

	String t = data.getProperty("itemType", "");
	if (!canAddItemOfType(t))
	{
		if (showWarning) NLOGWARNING(niceName, "Can't paste data from clipboard : data is of wrong type (\"" + t + "\").");
		return nullptr;
	}

	int sIndex = items.indexOf(InspectableSelectionManager::mainSelectionManager->getInspectableAs<T>());
	if(sIndex >= 0) data.getDynamicObject()->setProperty("index", sIndex+1);
	return addItemFromData(data);
}

template<class T>
bool BaseManager<T>::canAddItemOfType(const String & typeToCheck)
{
	return typeToCheck == itemDataType;
}

template<class T>
void BaseManager<T>::loadItemsData(var data)
{
	if (data == var()) return;
	Array<var> * itemsData = data.getProperty("items", var()).getArray();
	if (itemsData == nullptr) return;

	for (auto &td : *itemsData)
	{
		String n = td.getProperty("niceName", "");
		BaseItem * i = getItemWithName(n, true);
		if (i != nullptr) i->loadJSONData(td);
	}
}

template<class T>
Array<UndoableAction *> BaseManager<T>::getRemoveItemUndoableAction(T * item)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	Array<UndoableAction *> a;
	a.add(new RemoveItemAction(this, item));
	return a;
}

template<class T>
Array<UndoableAction*> BaseManager<T>::getRemoveItemsUndoableAction(Array<T*> itemsToRemove)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	Array<UndoableAction *> a;
	a.add(new RemoveItemsAction(this, itemsToRemove));
	return a;
}

template<class T>
void BaseManager<T>::removeItems(Array<T *> itemsToRemove, bool addToUndo)
{
	if (addToUndo)
	{
		Array<UndoableAction *> a = getRemoveItemsUndoableAction(itemsToRemove);
		UndoMaster::getInstance()->performActions("Remove " + String(itemsToRemove.size()) + " items", a);
		return;
	}

	baseManagerListeners.call(&Listener::itemsRemoved, itemsToRemove);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REMOVED));

	for (auto &i : itemsToRemove) removeItem(i, false, false);
}

template<class T>
void BaseManager<T>::removeItem(T * item, bool addToUndo, bool notify)
{
	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			BaseItem * bi = static_cast<BaseItem *>(item);
			UndoMaster::getInstance()->performActions("Remove " + bi->getTypeString(), getRemoveItemUndoableAction(item));
			return;
		}
	}


	items.removeObject(item, false);
	BaseItem * bi = static_cast<BaseItem *>(item);
	removeChildControllableContainer(bi);
	removeItemInternal(item);
	bi->removeBaseItemListener(this);

	if (notify)
	{
		baseManagerListeners.call(&BaseManager::Listener::itemRemoved, item);
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_REMOVED, item));

	}

	bi->clearItem();
	delete item;
}

template<class T>
void BaseManager<T>::reorderItems()
{
	if (comparator.compareFunc == nullptr) return;
	controllableContainers.sort(comparator);
	baseManagerListeners.call(&Listener::itemsReordered);
	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}

template<class T>
inline T * BaseManager<T>::getItemWithName(const String & itemShortName, bool searchItemWithNiceNameToo)
{
	for (auto &t : items)
	{
		if (((BaseItem *)t)->shortName == itemShortName) return t;
		else if (searchItemWithNiceNameToo && ((BaseItem *)t)->niceName == itemShortName) return t;
	}

	return nullptr;
}

template<class T>
void BaseManager<T>::clear()
{
	while (items.size() > 0) removeItem(items[0], false);
}

template<class T>
void BaseManager<T>::askForRemoveBaseItem(BaseItem * item)
{
	removeItem(static_cast<T*>(item));
}

template<class T>
void BaseManager<T>::askForDuplicateItem(BaseItem * item)
{
	if (!userCanAddItemsManually) return;
	var data = item->getJSONData();
	data.getDynamicObject()->setProperty("index", items.indexOf(static_cast<T *>(item))+1);
	addItemFromData(data);
}

template<class T>
void BaseManager<T>::askForPaste()
{
	addItemFromClipboard();
}

template<class T>
void BaseManager<T>::askForMoveBefore(BaseItem * i)
{
	int index = items.indexOf(static_cast<T *>(i));
	if (index == 0) return;
	items.swap(index, index - 1);
	reorderItems();
}

template<class T>
void BaseManager<T>::askForMoveAfter(BaseItem * i)
{
	int index = items.indexOf(static_cast<T *>(i));
	if (index == items.size() -1) return;
	items.swap(index, index + 1);
	reorderItems();
}

template<class T>
var BaseManager<T>::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	var itemsData = var();
	for (auto &t : items)
	{
		if(t->isSavable) itemsData.append(t->getJSONData());
	}

	if (itemsData.size() > 0) data.getDynamicObject()->setProperty("items", itemsData);

	return data;
}

template<class T>
void BaseManager<T>::loadJSONDataInternal(var data)
{
	clear();
	loadJSONDataManagerInternal(data);
}

template<class T>
void BaseManager<T>::loadJSONDataManagerInternal(var data)
{
	Array<var> * itemsData = data.getProperty("items", var()).getArray();
	if (itemsData == nullptr) return;

	for (auto &td : *itemsData)
	{
		addItemFromData(td, false);
	}

}

template<class T>
void BaseManager<T>::updateLiveScriptObjectInternal(DynamicObject *)
{
	ControllableContainer::updateLiveScriptObjectInternal();

	var itemsArray = var();
	for (auto &t : items) itemsArray.append(t->getScriptObject());
	liveScriptObject->setProperty("items", itemsArray);
}

template<class T>
inline PopupMenu BaseManager<T>::getItemsMenu(int startID)
{
	PopupMenu menu;
	int numValues = items.size();
	for (int j = 0; j < numValues; j++)
	{
		T * c = items[j];
		menu.addItem(startID + j, c->niceName);
	}
	return menu;
}

template<class T>
inline T * BaseManager<T>::getItemForMenuResultID(int id, int startID)
{
	return items[id - startID];
}


template<class T>
InspectableEditor * BaseManager<T>::getEditor(bool isRoot)
{
	return new GenericManagerEditor<T>(this, isRoot);

}


#endif  // BASEMANAGER_H_INCLUDED


