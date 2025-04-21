#include "JuceHeader.h"

BaseManager::BaseManager(const juce::String& name, bool canHaveGroups) :
	EnablingControllableContainer(name, true),
	canHaveGroups(canHaveGroups),
	userCanAddItemsManually(true),
	selectItemWhenCreated(false),
	autoReorderOnAdd(false),
	isManipulatingMultipleItems(false),
	clipboardCopyOffset(0, 0),
	viewOffset(0, 0),
	viewZoom(1.0f),
	showItemsInEditor(true)
{
	canBeCopiedAndPasted = true;
}

BaseItem* BaseManager::addItem(BaseItem* item, juce::var data, bool addToUndo, bool notify)
{

	if (item == nullptr) return nullptr; //could not create here

	if (addToUndo && !UndoMaster::getInstance()->isPerforming)
	{
		if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
		{
			//UndoMaster::getInstance()->performAction("Add " + item->niceName, new AddItemAction(this, item, data));
			return item;
		}
	}
	item->addBaseItemListener(this);
	if (!data.isVoid()) item->loadJSONData(data);
	addChildControllableContainer(item, false, baseItems.indexOf(item), notify);

	addItemManagerInternal(item, data, notify);

	if (notify) notifyItemAdded(item);
		//to move to notify
		//managerListeners.call(&ManagerListener::itemAdded, item);
		//managerNotifier.addMessage(new ManagerEvent(ManagerEvent::ITEM_ADDED, item));

	if (juce::MessageManager::getInstance()->existsAndIsLockedByCurrentThread())
	{
		if (selectItemWhenCreated && !isCurrentlyLoadingData && !isManipulatingMultipleItems) item->selectThis();
	}

	return item;
}

juce::Array<BaseItem*> BaseManager::getBaseItems(bool recursive, bool includeDisabled, bool includeGroups) const
{
	juce::Array<BaseItem*> result;
	for (auto& i : baseItems)
	{
		if (i->canBeDisabled && (i->enabled->boolValue() || includeDisabled)) continue;

		if (i->isGroup)
		{
			if (includeGroups) result.add((BaseItem*)i);
			if (recursive)
			{
				auto* group = (BaseItemGroup*)i;
				if (group) group->baseManager->getBaseItems(recursive, includeGroups);
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
