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
public : 
	BaseManager<T>(const String &name);
	virtual ~BaseManager<T>();

	OwnedArray<T> items;

	//Factory
	Factory<T> * managerFactory;
	
	bool userCanAddItemsManually;
	bool selectItemWhenCreated;

	virtual T * createItem(); //to override if special constructor to use
	virtual void addItemFromData(var data, bool fromUndoableAction = false); //to be overriden for specific item creation (from data)
	virtual void addItemFromClipboard();

	virtual void loadItemsData(var data);

	T* addItem(T * = nullptr, var data = var(), bool fromUndoableAction = false); //if data is not empty, load data
	T* addItem(const Point<float> initialPosition);

	void removeItem(T *, bool fromUndoableAction = false);


	virtual void reorderItems(); //to be overriden if needed

	//to override for specific handling like adding custom listeners, etc.
	virtual void addItemInternal(T *, var data) {}
	virtual void removeItemInternal(T *) {}


	T * getItemWithName(const String &itemShortName, bool searchNiceNameToo = false);

	void clear();
	void askForRemoveBaseItem(BaseItem * item) override;
	void askForDuplicateItem(BaseItem * item) override;
	void askForPaste() override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	DynamicObject * createScriptObject(DynamicObject * = nullptr) override;

	PopupMenu getItemsMenu(int startID);
	T * getItemForMenuResultID(int id, int startID);

	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void itemAdded(T *) {}
		virtual void itemRemoved(T *) {}
		virtual void itemsReordered() {}
	};

	ListenerList<Listener> baseManagerListeners;
	void addBaseManagerListener(Listener* newListener) { baseManagerListeners.add(newListener); }
	void removeBaseManagerListener(Listener* listener) { baseManagerListeners.remove(listener); }
	 
	InspectableEditor * getEditor(bool /*isRoot*/) override;



	//UNDO MANAGER
    /*
	class ManagerBaseAction :
		public UndoableAction
	{
	public:
		ManagerBaseAction(BaseManager * manager, var _data = var()) :
			managerRef(manager),
			managerControlAddress(manager->getControlAddress()),
			data(_data)
		{}

		String managerControlAddress;
		var data;
		BaseManager * managerRef;

		BaseManager<T> * getManager() { 
			if (managerRef != nullptr) return managerRef;
			else return nullptr;// static_cast<BaseManager<T>>(Engine::mainEngine->getControllableContainerForAddress(managerControlAddress), true);
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
			if (i != nullptr)
			{
				itemShortName = static_cast<BaseItem *>(itemRef)->shortName;
				DBG("ItemBase Action :: " << static_cast<BaseItem *>(itemRef)->niceName);
			}
		}

		T * itemRef;
		String itemShortName;

		T * getItem()
		{
			if (itemRef != nullptr) return itemRef;
			else
			{
				BaseManager * m = getManager();
				if (m != nullptr) return m->getItemWithName(itemShortName);
				return nullptr;
			}
		}
	};

	class AddItemAction :
		public ItemBaseAction
	{
	public:
		AddItemAction(BaseManager * m, T * i, var data = var()) : ItemBaseAction(m, i, data) { 
			DBG("new Add Item action");
		}

		bool perform() override
		{
			BaseManager * m = getManager();
			if (m == nullptr)
			{
				DBG("Perform :: Manager is null, doing nothing");
				return false;
			}

			T * item = getItem();
			if (item != nullptr)
			{
				m->addItem(item, data, true);
			}
			else
			{
				m->addItemFromData(data,true);
			}
			return true;
		}

		bool undo() override
		{
			T * s = getItem();
			if (s == nullptr) return false;
			data = s->getJSONData();
			getManager()->removeItem(s, true);
			itemRef = nullptr;
			return true;
		}
	};

	class RemoveItemAction :
		public ItemBaseAction
	{
	public:
		RemoveItemAction(BaseManager * m, T * i, var data = var()) : ItemBaseAction(m, i, data) {
			DBG("New remove Item action ");
		}

		bool perform() override
		{
			T * s = getItem();
			if (s == nullptr) return false;
			
			data = s->getJSONData();
			getManager()->removeItem(s, true);
			itemRef = nullptr;
			return true;
		}

		bool undo() override
		{
			BaseManager * m = getManager();
			if (m == nullptr) return false;
			m->addItemFromData(data, true);
			return true;
		}
	};
     */
    
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseManager<T>)
};


template<class T>
BaseManager<T>::BaseManager(const String & name) :
	ControllableContainer(name),
	managerFactory(nullptr),
	selectItemWhenCreated(true),
	userCanAddItemsManually(true)
{
	setCanHavePresets(false);
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
T * BaseManager<T>::addItem(T * item, var data, bool /*fromUndoableAction*/)
{
	/*
	if (!fromUndoableAction && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performAction("Add item", new AddItemAction(this, item, data));
			return nullptr;
		}
	}
	*/	

	jassert(items.indexOf(item) == -1); //be sure item is no here alread
	
	if (item == nullptr) item = createItem();

	items.add(item);
	BaseItem * bi = static_cast<BaseItem *>(item);
	addChildControllableContainer(bi);
	bi->addBaseItemListener(this);
	
	if (!data.isVoid())
	{
		bi->loadJSONData(data);
		bi->nameParam->setValue(getUniqueNameInContainer(bi->niceName)); //force setting a unique name if already taken
	}

	
	addItemInternal(item, data);
	
	baseManagerListeners.call(&BaseManager::Listener::itemAdded, item);
	reorderItems();

	if (selectItemWhenCreated) bi->selectThis();

	return item;
}

template<class T>
T * BaseManager<T>::addItem(const Point<float> initialPosition)
{
	T * i = createItem();
	i->viewUIPosition->setPoint(initialPosition);
	addItem(i);
	return i;
}

//if data is not empty, load data
template<class T>
void BaseManager<T>::addItemFromData(var data, bool fromUndoableAction) 
{ 
	if (managerFactory != nullptr)
	{
		String type = data.getProperty("type", "");
		if (type.isEmpty()) return;
		T * i = managerFactory->create(type);
		if (i != nullptr) addItem(i, data, fromUndoableAction);
	} else
	{
		addItem(createItem(), data, fromUndoableAction);
	}
}

template<class T>
void BaseManager<T>::addItemFromClipboard()
{
	if (!userCanAddItemsManually) return;
	String s = SystemClipboard::getTextFromClipboard();
	var data = JSON::parse(s);
	if (data.isVoid()) return;

	if (data.getProperty("itemType", var()).isVoid()) return;
	addItemFromData(data);
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
void BaseManager<T>::removeItem(T * item, bool /*fromUndoableAction*/)
{
	/*
	if (!fromUndoableAction && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			UndoMaster::getInstance()->performAction("Remove item", new RemoveItemAction(this, item));
			return;
		}
	}
	*/

	items.removeObject(item, false);
	BaseItem * bi = static_cast<BaseItem *>(item);
	removeChildControllableContainer(bi);
	removeItemInternal(item);
	bi->removeBaseItemListener(this);

	baseManagerListeners.call(&BaseManager::Listener::itemRemoved, item);

	delete item;
}

template<class T>
void BaseManager<T>::reorderItems()
{
	baseManagerListeners.call(&Listener::itemsReordered);
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
	while (items.size() > 0) removeItem(items[0]);
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
	data.getDynamicObject()->setProperty("items", itemsData);

	return data;
}

template<class T>
void BaseManager<T>::loadJSONDataInternal(var data)
{ 
	clear();
	Array<var> * itemsData = data.getProperty("items",var()).getArray();
	if (itemsData == nullptr) return;
	for (auto &td : *itemsData)
	{ 
		addItemFromData(td);
	}
}

template<class T>
DynamicObject * BaseManager<T>::createScriptObject(DynamicObject *)
{
	DynamicObject * o = ControllableContainer::createScriptObject();
	var itemsArray = var();
	for (auto &t : items) itemsArray.append(t->createScriptObject());
	o->setProperty("items", itemsArray);
	return o;
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
	BaseManagerUI<BaseManager<T>, T, BaseItemUI<T>> * bui = new  BaseManagerUI<BaseManager<T>, T, BaseItemUI<T>>(niceName, this,false);
	bui->drawContour = true;
	return new GenericComponentEditor(this, bui, isRoot);
}


#endif  // BASEMANAGER_H_INCLUDED


