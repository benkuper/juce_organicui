#include "JuceHeader.h"
#include "BaseManager.h"

BaseManager::BaseManager(const String& name, bool canHaveGroups) :
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
}

BaseManager::~BaseManager()
{
	clear();
}

void BaseManager::clear()
{
	isClearing = true;
	//const ScopedLock lock(baseItems.getLock());
	while (baseItems.size() > 0) removeItem(baseItems[0], false);
	isClearing = false;

#if ORGANICUI_USE_WEBSERVER
	if (notifyRemoteControlOnClear && !isCurrentlyLoadingData && isAttachedToRoot()) OSCRemoteControl::getInstance()->sendPathChangedFeedback(getControlAddress());
#endif
}

BaseItem* BaseManager::addItem(BaseItem* item, var data, bool addToUndo, bool notify)
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
	addChildControllableContainer(item, false, baseItems.indexOf(item), notify);

	int targetIndex = data.getProperty("index", -1);
	if (targetIndex != -1) baseItems.insert(targetIndex, item);
	else baseItems.add(item);

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

Array<BaseItem*> BaseManager::addItems(Array<BaseItem*> itemsToAdd, var data, bool addToUndo, bool notify)
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
		addItem(itemsToAdd[i], data.isArray() ? data[i] : var(), false, false);
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
BaseItem* BaseManager::addItemFromData(var data, bool addToUndo)
{
	BaseItem* item = createItemFromData(data);
	if (item == nullptr) return nullptr;
	return addItem(item, data, addToUndo, true);
}

Array<BaseItem*> BaseManager::addItemsFromData(var data, bool addToUndo)
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

Array<BaseItem*> BaseManager::addItemsFromClipboard(bool showWarning)
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

	var itemsData = data.getProperty("items", var());
	int sIndex = baseItems.indexOf(InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>());
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
bool BaseManager::canAddItemOfType(const String& typeToCheck)
{
	if (typeToCheck == itemDataType) return true;
	return false;
}


void BaseManager::loadItemsData(var data)
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

BaseItem* BaseManager::removeItem(BaseItem* item, bool addToUndo, bool notify, bool returnItem)
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

	baseItems.removeObject(item, false);

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


void BaseManager::removeItems(Array<BaseItem*> itemsToRemove, bool addToUndo, bool notify)
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

UndoableAction* BaseManager::getAddBaseItemUndoableAction(BaseItem* item, var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	jassert(baseItems.indexOf(item) == -1); //be sure item is no here already
	if (item == nullptr) item = createItem();
	return new AddItemAction(this, item, data);
}

UndoableAction* BaseManager::getAddBaseItemsUndoableAction(Array<BaseItem*> _items, var data)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	if (_items.size() == 0) return nullptr;
	return new AddItemsAction(this, _items, data);
}

Array<UndoableAction*> BaseManager::getRemoveBaseItemUndoableAction(BaseItem* item)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	Array<UndoableAction*> a;
	a.add(new RemoveItemAction(this, item));
	return a;
}

Array<UndoableAction*> BaseManager::getRemoveBaseItemsUndoableAction(Array<BaseItem*> _items)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;
	Array<UndoableAction*> a;
	a.add(new RemoveItemsAction(this, _items));
	return a;
}


void BaseManager::setItemIndex(BaseItem* item, int newIndex, bool addToUndo)
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


	newIndex = jlimit(0, baseItems.size() - 1, newIndex);
	int index = baseItems.indexOf(item);
	if (index == -1 || index == newIndex) return;

	//items.getLock().enter();
	baseItems.move(index, newIndex);
	controllableContainers.move(index, newIndex);
	//items.getLock().exit();

	notifyItemsReordered();
}

Array<UndoableAction*> BaseManager::getSetItemIndexUndoableAction(BaseItem* item, int newIndex)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return nullptr;

	Array<UndoableAction*> a;
	a.add(new MoveItemAction(this, item, this->baseItems.indexOf(item), newIndex));
	return a;
}



BaseItem* BaseManager::getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo, bool searchWithLowerCaseIfNotFound)
{
	//const ScopedLock lock(baseItems.getLock());

	for (auto& t : baseItems)
	{
		if (((BaseItem*)t)->shortName == itemShortName) return t;
		else if (searchNiceNameToo && ((BaseItem*)t)->niceName == itemShortName) return t;

	}

	if (searchWithLowerCaseIfNotFound)
	{
		for (auto& t : baseItems)
		{
			if (((BaseItem*)t)->shortName.toLowerCase() == itemShortName.toLowerCase()) return t;
		}
	}

	return nullptr;
}

String BaseManager::getItemPath(BaseItem* item)
{
	return item->getControlAddress(this);
}

BaseItem* BaseManager::getItemWithPath(const juce::String& relativePath)
{
	if (BaseItem* i = dynamic_cast<BaseItem*>(getControllableContainerForAddress(relativePath))) return i;
	return nullptr;
}

Array<BaseItem*> BaseManager::getBaseItems(bool recursive, bool includeDisabled, bool includeGroups) const
{
	Array<BaseItem*> result;
	for (auto& i : baseItems)
	{
		if (i->canBeDisabled && !i->enabled->boolValue() && !includeDisabled) continue;

		if (i->isGroup)
		{
			if (includeGroups) result.add((BaseItem*)i);
			if (recursive)
			{
				if (auto group = dynamic_cast<BaseItemGroup*>(i))
					result.addArray(group->baseManager->getBaseItems(recursive, includeDisabled, includeGroups));
			}
		}
		else
		{
			result.add(i);
		}
	}

	return result;
}


void BaseManager::callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func)
{
	for (auto& i : baseItems)
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

void BaseManager::handleAddFromRemoteControl(var data)
{
	if (!userCanAddItemsManually) return;
	addItemFromData(data);
}


void BaseManager::askForRemoveBaseItem(BaseItem* item)
{
	removeItem(item);
}

void BaseManager::askForDuplicateItem(BaseItem* item)
{
	if (!userCanAddItemsManually) return;
	var data = item->getJSONData();
	data.getDynamicObject()->setProperty("index", baseItems.indexOf(static_cast<BaseItem*>(item)) + 1);
	addItemFromData(data);
}
void BaseManager::askForPaste()
{
	addItemsFromClipboard();
}
void BaseManager::askForMoveBefore(BaseItem* item)
{
	setItemIndex(item, jmax(baseItems.indexOf(item) - 1, 0));
	//	int index = baseItems.indexOf(static_cast<T *>(i));
	//	if (index == 0) return;
	//	baseItems.swap(index, index - 1);
	//	controllableContainers.swap(index, index - 1);
	//
	//	managerListeners.call(&ManagerListener::itemsReordered);
	//	managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}
void BaseManager::askForMoveAfter(BaseItem* item)
{
	setItemIndex(item, jmin(baseItems.indexOf(item) + 1, baseItems.size() - 1));
	//int index = baseItems.indexOf(static_cast<T *>(i));
	//if (index == baseItems.size() -1) return;
	//baseItems.swap(index, index + 1);
	//controllableContainers.swap(index, index+1);

	//managerListeners.call(&ManagerListener::itemsReordered);
	//managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEMS_REORDERED));
}
void BaseManager::askForSelectAllItems(bool addToSelection)
{
	int numItems = baseItems.size();
	if (!addToSelection) selectionManager->clearSelection(numItems == 0);
	else deselectThis(numItems == 0);

	if (numItems > 1) for (int i = 0; i < numItems; ++i) baseItems[i]->selectThis(true, i == numItems - 1); //only notify on last
	else if (numItems > 0) baseItems.getFirst()->selectThis(addToSelection, true);
}

void BaseManager::askForSelectPreviousItem(BaseItem* item, bool addToSelection)
{
	int index = baseItems.indexOf(item);
	if (index <= 0) return;
	baseItems[index - 1]->selectThis(addToSelection);
}
void BaseManager::askForSelectNextItem(BaseItem* item, bool addToSelection)
{
	int index = baseItems.indexOf(item);
	if (index == -1 || index >= baseItems.size() - 1) return;
	baseItems[index + 1]->selectThis(addToSelection);
}

void BaseManager::onContainerParameterChanged(Parameter* p)
{
	EnablingControllableContainer::onContainerParameterChanged(p);
	if (p == showSnapGrid || p == snapGridSize)
	{
		notifyAsync(BaseManagerEvent::NEEDS_UI_UPDATE);
	}
}

var BaseManager::getExportSelectionData()
{
	var data;

	for (auto& t : baseItems)
	{
		if (t->isSavable && t->isSelected) data.append(t->getJSONData());
	}

	return data;
}

var BaseManager::getJSONData(bool includeNonOverriden)
{
	var data = ControllableContainer::getJSONData(includeNonOverriden);
	var itemsData = var();
	//baseItems.getLock().enter();
	for (auto& t : baseItems)
	{
		if (t->isSavable) itemsData.append(t->getJSONData(includeNonOverriden));
	}
	//baseItems.getLock().exit();

	if (itemsData.size() > 0) data.getDynamicObject()->setProperty("items", itemsData);

	var vData;
	vData.append(viewOffset.x);
	vData.append(viewOffset.y);
	data.getDynamicObject()->setProperty("viewOffset", vData);
	data.getDynamicObject()->setProperty("viewZoom", viewZoom);

	return data;
}
void BaseManager::loadJSONDataInternal(var data)
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
void BaseManager::loadJSONDataManagerInternal(var data)
{
	var itemsData = data.getProperty("items", var());
	if (itemsData.isVoid()) return;

	addItemsFromData(itemsData, false);
}

var BaseManager::getRemoteControlData()
{
	if (isClearing || isCurrentlyLoadingData) return var(new DynamicObject());
	return ControllableContainer::getRemoteControlData();
}

void BaseManager::getRemoteControlDataInternal(var& data)
{
	data.getDynamicObject()->setProperty("TYPE", "Manager");
	data.getDynamicObject()->setProperty("BASE_TYPE", itemDataType);
}

String BaseManager::getScriptTargetString()
{
	return "[" + niceName + " : Manager(" + itemDataType + ")]";
}

BaseManager::BaseManagerEvent::BaseManagerEvent(Type t, Array<BaseItem*> iList, bool fromChildGroup) :
	type(t),
	fromChildGroup(fromChildGroup)
{
	for (auto& i : iList) if (i != nullptr) this->baseItems.add(i);
}


//ACTIONS
BaseManager::ManagerBaseAction::ManagerBaseAction(BaseManager* manager, var _data) :
	managerControlAddress(manager->getControlAddress()),
	data(_data),
	managerRef(manager)
{
}

BaseManager* BaseManager::ManagerBaseAction::getManager() {
	if (managerRef != nullptr && !managerRef.wasObjectDeleted()) return dynamic_cast<BaseManager*>(managerRef.get());
	else if (Engine::mainEngine != nullptr)
	{
		ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(managerControlAddress, true);
		if (cc != nullptr) return dynamic_cast<BaseManager*>(cc);
	}

	return nullptr;
}

BaseManager::ItemBaseAction::ItemBaseAction(BaseManager* m, BaseItem* i, var data) :
	ManagerBaseAction(m, data),
	baseItem(i),
	itemIndex(0)
{
	if (baseItem != nullptr)
	{
		this->itemShortName = this->baseItem->shortName;
		this->itemIndex = data.getProperty("index", m->baseItems.indexOf(baseItem));

	}
}

BaseItem* BaseManager::ItemBaseAction::getItem()
{
	if (baseItem != nullptr && !baseItem.wasObjectDeleted()) return baseItem.get();
	else
	{
		BaseManager* m = this->getManager();
		if (m != nullptr) return m->getItemWithName(itemShortName);
	}

	return nullptr;
}

BaseManager::AddItemAction::AddItemAction(BaseManager* m, BaseItem* i, var data) : ItemBaseAction(m, i, data) {
}

bool BaseManager::AddItemAction::perform()
{
	BaseManager* m = this->getManager();
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

bool BaseManager::AddItemAction::undo()
{
	BaseItem* s = this->getItem();
	if (s == nullptr) return false;
	this->data = s->getJSONData();
	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(s, false);
	this->baseItem = nullptr;
	return true;
}

BaseManager::RemoveItemAction::RemoveItemAction(BaseManager* m, BaseItem* i, var data) : ItemBaseAction(m, i, data)
{

}

bool BaseManager::RemoveItemAction::perform()
{

	if (this->baseItem == nullptr) return false;

	this->data = this->baseItem->getJSONData();
	if (this->data.getDynamicObject() == nullptr) return false;

	this->data.getDynamicObject()->setProperty("index", this->itemIndex);

	this->getManager()->removeItem(this->baseItem, false);
	this->baseItem = nullptr;
	return true;
}

bool BaseManager::RemoveItemAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;
	this->baseItem = m->addItemFromData(this->data, false);
	return true;
}


BaseManager::MoveItemAction::MoveItemAction(BaseManager* m, BaseItem* i, int prevIndex, int newIndex) :
	ItemBaseAction(m, i),
	prevIndex(prevIndex),
	newIndex(newIndex)
{

}

bool BaseManager::MoveItemAction::perform()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	BaseItem* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, newIndex, false);
	return true;
}

bool BaseManager::MoveItemAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	BaseItem* item = this->getItem();
	if (item == nullptr) return false;

	m->setItemIndex(item, prevIndex, false);
	return true;
}


BaseManager::ItemsBaseAction::ItemsBaseAction(BaseManager* m, Array<BaseItem*> iList, var data) :
	ManagerBaseAction(m, data)
{
	if (data.isVoid())
	{
		std::sort(iList.begin(), iList.end(),
			[m](const auto& lhs, const auto& rhs) { return m->baseItems.indexOf(lhs) < m->baseItems.indexOf(rhs); });
	}

	for (auto& i : iList)
	{
		baseItems.add(i);
		itemsShortName.add(i != nullptr ? i->shortName : "");
	}
}

Array<BaseItem*> BaseManager::ItemsBaseAction::getItems()
{
	Array<BaseItem*> iList;
	int index = 0;
	for (auto& i : baseItems)
	{
		if (i != nullptr && !i.wasObjectDeleted())
		{
			BaseItem* ti = dynamic_cast<BaseItem*>(i.get());
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

BaseManager::AddItemsAction::AddItemsAction(BaseManager* m, Array<BaseItem*> iList, var data) :
	ItemsBaseAction(m, iList, data)
{
}

bool BaseManager::AddItemsAction::perform()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	Array<BaseItem*> iList = this->getItems();
	if (!iList.isEmpty()) m->addItems(iList, this->data, false);
	else if (!this->data.isVoid())
	{
		Array<BaseItem*> newList = m->addItemsFromData(this->data, false);
		this->baseItems.clear();
		for (auto& i : newList) this->baseItems.add(i);
	}


	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}

bool BaseManager::AddItemsAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr)
	{
		this->baseItems.clear();
		return false;
	}

	Array<BaseItem*> iList = this->getItems();
	this->data = var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->baseItems.indexOf(i));
			this->data.append(d);
		}
	}
	m->removeItems(iList, false);
	return true;
}

BaseManager::RemoveItemsAction::RemoveItemsAction(BaseManager* m, Array<BaseItem*> iList) : ItemsBaseAction(m, iList) {
}

bool BaseManager::RemoveItemsAction::perform()
{
	BaseManager* m = this->getManager();
	if (m == nullptr)
	{
		this->baseItems.clear();
		return false;
	}

	Array<BaseItem*> iList = this->getItems();
	this->data = var();
	for (auto& i : iList)
	{
		if (i != nullptr)
		{
			var d = i->getJSONData();
			d.getDynamicObject()->setProperty("index", m->baseItems.indexOf(i));
			this->data.append(d);
		}
	}

	m->removeItems(iList, false);
	return true;
}

bool BaseManager::RemoveItemsAction::undo()
{
	BaseManager* m = this->getManager();
	if (m == nullptr) return false;

	Array<BaseItem*> iList = m->addItemsFromData(this->data, false);

	this->itemsShortName.clear();
	for (auto& i : iList) this->itemsShortName.add(i != nullptr ? i->shortName : "");
	return true;
}
