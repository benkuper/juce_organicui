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
	public ControllableContainer,
	public BaseItem::Listener
{
public:
	BaseManager<T>(const String &name);
	virtual ~BaseManager<T>();

	OwnedArray<T> items;

	//Factory
	Factory<T> * managerFactory;

	bool userCanAddItemsManually;
	bool selectItemWhenCreated;

	virtual T * createItem(); //to override if special constructor to use
	virtual T * addItemFromData(var data, bool addToUndo = true); //to be overriden for specific item creation (from data)
	virtual T * addItemFromClipboard();

	virtual void loadItemsData(var data);


	UndoableAction * getAddItemUndoableAction(T * item = nullptr, var data = var());

	T * addItem(T * item = nullptr, var data = var(), bool addToUndo = true, bool notify = true); //if data is not empty, load data
	T * addItem(const Point<float> initialPosition, bool addToUndo = true, bool notify = true);
	Array<UndoableAction *> addItems(Array<T *> items, bool addToUndo = true, bool onlyReturnAction = false);


	UndoableAction * getRemoveItemUndoableAction(T * item);
	void removeItems(Array<T *> items, bool addToUndo = true);
	void removeItem(T * item, bool addToUndo = true, bool notify = true);

	virtual void reorderItems(); //to be overriden if needed

	//to override for specific handling like adding custom listeners, etc.
	virtual void addItemInternal(T *, var data) {}
	virtual void removeItemInternal(T *) {}


	T * getItemWithName(const String &itemShortName, bool searchNiceNameToo = false);

	virtual void clear();
	void askForRemoveBaseItem(BaseItem * item) override;
	void askForDuplicateItem(BaseItem * item) override;
	void askForPaste() override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

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

		ManagerEvent(Type t, Array<T *> i) : type(t), itemsRef(i) {}

		Type type;
		Array<WeakReference<Inspectable>> itemsRef;

		Array<T *> getItems() const
		{
			Array<T *> result;
			for (auto &i : itemsRef)
			{
				if (i != nullptr && !i.wasObjectDeleted()) result.add(i);
			}
			return result;
		}

		T * getItem(int index=0) const
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
			else
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
				itemShortName = dynamic_cast<BaseItem *>(s)->shortName;
			}
		}

		WeakReference<Inspectable> itemRef;
		String itemShortName;

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
			ManagerBaseAction(m, data),
			itemsRef(iList)
		{
			for (auto &i : itemsRef) itemsShortName.add(i != nullptr ? i->shortName : "");
		}

		Array<WeakReference<Inspectable>> itemsRef;
		StringArray itemsShortName;

		T * getItems()
		{
			Array<T *> items;
			int index = 0;
			for (auto &i : itemsRef)
			{
				if (i != nullptr && !i.wasObjectDeleted())
				{
					T * ti = dynamic_cast<T *>(itemRef.get());
					if (ti != nullptr) items.add(ti);
					else
					{
						BaseManager * m = this->getManager();
						if (m != nullptr)
						{
							T * ti = m->getItemWithName(itemShortName);
							if (ti != nullptr) items.add(ti);
						}
					}
				}

				return items;
			}
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

			Array<T *> items = this->getItems();
			m->addItems(items, false, false);

			this->itemsShortName.clear();
			for (auto &i : items) this->itemsShortName.add(i != nullptr ? i->shortName : ""); 
			return true;
		}

		bool undo() override
		{
			Array<T *> items = this->getItems();
			if (items == nullptr) return false;
			this->data = var();
			for (auto & i : items) if (i != nullptr) this->data.append(i->getJSONData());
			BaseManager * m = this->getManager();
			if(m != nullptr) m->removeItems(items, false);
			this->itemsRef.clear();
			return true;
		}
	};

	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseManager<T>)
};


template<class T>
BaseManager<T>::BaseManager(const String & name) :
	ControllableContainer(name),
	managerFactory(nullptr),
	userCanAddItemsManually(true),
	selectItemWhenCreated(true),
	managerNotifier(50)
{
	//setCanHavePresets(false);
	//hideInEditor = true;
	nameCanBeChangedByUser = false;
	isSelectable = false;
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


	items.add(item);
	bi->addBaseItemListener(this);

	if (!data.isVoid())
	{
		bi->loadJSONData(data);
	}

	//bi->setNiceName(bi->niceName); //force setting a unique name if already taken, after load data so if name is the same as another, will change here

	addChildControllableContainer(bi);

	addItemInternal(item, data);

	if (notify)
	{
		baseManagerListeners.call(&BaseManager::Listener::itemAdded, item);
		managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_ADDED, item));
	}

	reorderItems();

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
Array<UndoableAction *> BaseManager<T>::addItems(Array<T*> itemsToAdd, bool addToUndo, bool onlyReturnAction)
{
	Array<UndoableAction *> actions;

	//TODO replace with AddMultiItemsActions
	for (T * i : itemsToAdd) actions.add(getAddItemUndoableAction(i));
	if (!onlyReturnAction) UndoMaster::getInstance()->performActions("Add " + String(actions.size()) + " items", actions);

	return actions;
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
T * BaseManager<T>::addItemFromClipboard()
{
	if (!userCanAddItemsManually) return nullptr;
	String s = SystemClipboard::getTextFromClipboard();
	var data = JSON::parse(s);
	if (data.isVoid()) return nullptr;

	if (data.getProperty("itemType", var()).isVoid()) return nullptr;
	return addItemFromData(data);
}

template<class T>
void BaseManager<T>::loadItemsData(var data)
{
	if (data == var::null) return;
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
UndoableAction * BaseManager<T>::getRemoveItemUndoableAction(T * item)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	return new RemoveItemAction(this, item);
}

template<class T>
void BaseManager<T>::removeItems(Array<T*> itemsToRemove, bool addToUndo)
{
	Array<UndoableAction *> actions;
	for (auto &i : itemsToRemove) actions.add(getRemoveItemUndoableAction(i));
	UndoMaster::getInstance()->performActions("Remove " + String(actions.size()) + " actions", actions);
}

template<class T>
void BaseManager<T>::removeItem(T * item, bool addToUndo, bool notify)
{
	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performAction("Remove item", new RemoveItemAction(this, item));
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
	
	delete item;
}

template<class T>
void BaseManager<T>::reorderItems()
{
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
	addItemFromData(item->getJSONData());
}

template<class T>
void BaseManager<T>::askForPaste()
{
	addItemFromClipboard();
}

template<class T>
var BaseManager<T>::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	var itemsData = var();
	for (auto &t : items)
	{
		itemsData.append(t->getJSONData());
	}

	if (itemsData.size() > 0) data.getDynamicObject()->setProperty("items", itemsData);

	return data;
}

template<class T>
void BaseManager<T>::loadJSONDataInternal(var data)
{
	clear();
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


