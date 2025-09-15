/*
  ==============================================================================

	BaseItem.cpp
	Created: 28 Oct 2016 8:04:25pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

BaseItem::BaseItem(const String& name, bool _canBeDisabled, bool _canHaveScripts, bool isGroup) :
	EnablingControllableContainer(name.isEmpty() ? getTypeString() : name, _canBeDisabled),
	isGroup(isGroup),
	itemColor(nullptr),
	useCustomArrowKeysBehaviour(false),
	canHaveScripts(_canHaveScripts),
	userCanRemove(true),
	userCanDuplicate(true),
	askConfirmationBeforeRemove(true),
	isSavable(true),
	saveType(true),
	canBeReorderedInEditor(true),
	itemDataType("")
{
	//itemDataType = getTypeString();


	saveAndLoadName = true;
	nameCanBeChangedByUser = true;

	canBeCopiedAndPasted = true;

	if (canHaveScripts)
	{
		scriptManager.reset(new ScriptManager(this));
		//scriptManager->addManagerListener(this);
		addChildControllableContainer(scriptManager.get());
	}

	//For UI
	miniMode = addBoolParameter("MiniMode", "Set the mini mode", false);
	miniMode->hideInEditor = true;
	miniMode->hideInRemoteControl = true;
	miniMode->defaultHideInRemoteControl = true;

	listUISize = addFloatParameter("ListSize", "Size in list", 24, 0, 5000);
	listUISize->hideInEditor = true;
	listUISize->hideInRemoteControl = true;
	listUISize->defaultHideInRemoteControl = true;

	viewUIPosition = addPoint2DParameter("ViewUIPosition", "Position the view");
	//viewUIPosition->setBounds(-100000, -100000, 100000, 100000);
	viewUIPosition->hideInEditor = true;
	viewUIPosition->hideInRemoteControl = true;
	viewUIPosition->defaultHideInRemoteControl = true;

	viewUISize = addPoint2DParameter("ViewUISize", "Size in the view");
	//viewUISize->setBounds(30, 60, 10000, 10000);
	var defaultSize;
	defaultSize.append(200);
	defaultSize.append(200);
	viewUISize->defaultValue = defaultSize;
	viewUISize->resetValue();
	viewUISize->defaultValue = viewUISize->getValue();
	viewUISize->hideInEditor = true;
	viewUISize->hideInRemoteControl = true;
	viewUISize->defaultHideInRemoteControl = true;

	isUILocked = addBoolParameter("Locked", "if checked, item is locked in UI", false);
	isUILocked->hideInEditor = true;
	isUILocked->hideInRemoteControl = true;
	isUILocked->defaultHideInRemoteControl = true;

	scriptObject.getDynamicObject()->setMethod("getType", BaseItem::getTypeStringFromScript);
}

BaseItem::~BaseItem()
{
}

void BaseItem::clearItem()
{
	isClearing = true;
	if (canHaveScripts && scriptManager != nullptr) scriptManager->clear();
}

void BaseItem::duplicate()
{
	baseItemListeners.call(&BaseItemListener::askForDuplicateItem, this);
}

void BaseItem::copy()
{
	var data = getJSONData();
	data.getDynamicObject()->setProperty("itemType", itemDataType.isNotEmpty() ? itemDataType : getTypeString());
	SystemClipboard::copyTextToClipboard(JSON::toString(data));
	NLOG(niceName, "Copied to clipboard");
}

bool BaseItem::paste()
{
	//default behavior can be overriden
	baseItemListeners.call(&BaseItemListener::askForPaste);
	return true;
}

void BaseItem::selectAll(bool addToSelection)
{
	baseItemListeners.call(&BaseItemListener::askForSelectAllItems, addToSelection);
}

void BaseItem::selectPrevious(bool addToSelection)
{
	baseItemListeners.call(&BaseItemListener::askForSelectPreviousItem, this, addToSelection);
}

void BaseItem::selectNext(bool addToSelection)
{
	baseItemListeners.call(&BaseItemListener::askForSelectNextItem, this, addToSelection);
}

void BaseItem::moveBefore()
{
	baseItemListeners.call(&BaseItemListener::askForMoveBefore, this);
}

void BaseItem::moveAfter()
{
	baseItemListeners.call(&BaseItemListener::askForMoveAfter, this);
}

void BaseItem::remove()
{
	baseItemListeners.call(&BaseItemListener::askForRemoveBaseItem, this);
}

void BaseItem::handleRemoveFromRemoteControl()
{
	if (userCanRemove) MessageManager::callAsync([this]() {remove(); });
}

void BaseItem::setMovePositionReference(bool setOtherSelectedItems)
{
	setMovePositionReferenceInternal();

	if (setOtherSelectedItems)
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i == this || i->isUILocked->boolValue()) continue;
			i->setMovePositionReference(false);
		}
	}
}

void BaseItem::setMovePositionReferenceInternal()
{
	movePositionReference = getPosition();
}

void BaseItem::movePosition(Point<float> positionOffset, bool moveOtherSelectedItems)
{
	setPosition(movePositionReference + positionOffset);
	if (moveOtherSelectedItems)
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i == this || i->isUILocked->boolValue()) continue;
			i->movePosition(positionOffset, false);
		}
	}
}

void BaseItem::scalePosition(Point<float> positionOffset, bool moveOtherSelectedItems)
{
	if (moveOtherSelectedItems)
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();

		if (items.size() < 2) return;
		BaseItem* firstItemX = items[0];
		BaseItem* lastItemX = items[0];
		BaseItem* firstItemY = items[0];
		BaseItem* lastItemY = items[0];

		Point<float> minPosition = items[0]->movePositionReference;
		Point<float> maxPosition = items[0]->movePositionReference;
		for (auto& i : items)
		{
			if (i->isUILocked->boolValue()) continue;

			Point<float> t = i->movePositionReference;
			if (t.x < minPosition.x)
			{
				minPosition.setX(t.x);
				firstItemX = i;
			}
			else if (t.x > maxPosition.x)
			{
				maxPosition.setX(t.x);
				lastItemX = i;
			}

			if (t.y < minPosition.y)
			{
				minPosition.setY(t.y);
				firstItemY = i;
			}
			else if (t.y > maxPosition.y)
			{
				maxPosition.setY(t.y);
				lastItemY = i;
			}
		}

		Point<float> anchorPosition = Point<float>((this == firstItemX) ? maxPosition.x : minPosition.x, (this == firstItemY) ? maxPosition.y : minPosition.y);
		Point<float> anchorDiffToReference = movePositionReference - anchorPosition;

		if (anchorDiffToReference.isOrigin()) return;

		for (auto& i : items)
		{
			Point<float> positionScale = (i->movePositionReference - anchorPosition) / anchorDiffToReference;
			Point<float> targetOffset = positionOffset * positionScale;
			DBG(" > time scale " << positionScale.toString() << " / target offset " << targetOffset.toString());

			i->movePosition(targetOffset, false);
		}
	}
}

void BaseItem::setPosition(Point<float> position)
{
	viewUIPosition->setPoint(position);
}

Point<float> BaseItem::getPosition()
{
	return viewUIPosition->getPoint();
}

void BaseItem::addMoveToUndoManager(bool addOtherSelectedItems)
{
	Array<UndoableAction*> actions;

	int numItems = 1;
	if (!addOtherSelectedItems)
	{
		addUndoableMoveAction(actions);
	}
	else
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i->isUILocked->boolValue()) continue;
			i->addUndoableMoveAction(actions);
		}

		numItems = items.size();
	}

	UndoMaster::getInstance()->performActions("Move " + String(numItems) + " item" + String(numItems > 1 ? "s" : ""), actions);
}

void BaseItem::addUndoableMoveAction(Array<UndoableAction*>& arrayToAdd)
{
	arrayToAdd.addArray(viewUIPosition->setUndoablePoint(viewUIPosition->getPoint(), true));
}

void BaseItem::setSizeReference(bool setOtherSelectedItems)
{
	setSizeReferenceInternal();

	if (setOtherSelectedItems)
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i == this || i->isUILocked->boolValue()) continue;
			i->setSizeReference(false);
		}
	}
}

void BaseItem::setSizeReferenceInternal()
{
	sizeReference = getItemSize();
}

void BaseItem::resizeItem(Point<float> sizeOffset, bool resizeOtherSelectedItems)
{
	setItemSize(sizeReference + sizeOffset);

	if (resizeOtherSelectedItems)
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i == this || i->isUILocked->boolValue()) continue;
			i->resizeItem(sizeOffset, false);
		}
	}
}

void BaseItem::setItemSize(Point<float> size)
{
	viewUISize->setPoint(size);
}

Point<float> BaseItem::getItemSize()
{
	return viewUISize->getPoint();
}

void BaseItem::addResizeToUndoManager(bool addOtherSelectedItems)
{
	Array<UndoableAction*> actions;

	int numItems = 1;
	if (!addOtherSelectedItems)
	{
		addUndoableResizeAction(actions);
	}
	else
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i->isUILocked->boolValue()) continue;
			i->addUndoableResizeAction(actions);
		}

		numItems = items.size();
	}

	UndoMaster::getInstance()->performActions("Resize " + String(numItems) + " item" + String(numItems > 1 ? "s" : ""), actions);
}

void BaseItem::addUndoableResizeAction(Array<UndoableAction*>& arrayToAdd)
{
	arrayToAdd.addArray(viewUISize->setUndoablePoint(viewUISize->getPoint(), true));
}

void BaseItem::onContainerParameterChanged(Parameter* p)
{
	if (canHaveScripts)
	{
		Array<var> args;
		args.add(p->getScriptObject());
		scriptManager->callFunctionOnAllScripts("localParamChanged", args);
	}

	onContainerParameterChangedInternal(p);
}

void BaseItem::onContainerTriggerTriggered(Trigger* t)
{
	if (canHaveScripts)
	{
		Array<var> args;
		args.add(t->getScriptObject());
		scriptManager->callFunctionOnAllScripts("localParamChanged", args);
	}
}

void BaseItem::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	onControllableFeedbackUpdateInternal(cc, c);
}

//void BaseItem::itemAdded(Script* script)
//{
//	script->warningResolveInspectable = this;
//}
//
//void BaseItem::itemsAdded(Array<Script*> scripts)
//{
//	for (auto& script : scripts) script->warningResolveInspectable = this;
//
//}

juce::StringArray BaseItem::getBreadCrumb()
{
	StringArray result;
	result.add(niceName);
	//BaseItemGroup* g = ControllableUtil::findParentAs<BaseItemGroup>(this, 2);

	//while (g != nullptr)
	//{
	//	if (BaseItem* bi = dynamic_cast<BaseItem*>(g))
	//	{
	//		result.insert(0, bi->niceName);
	//		g = ControllableUtil::findParentAs<BaseItemGroup>(bi, 2);
	//	}
	//	else break;
	//}

	return result;
}

void BaseItem::setHasCustomColor(bool value)
{
	if (value)
	{
		if (itemColor == nullptr)
		{
			itemColor = addColorParameter("Color", "The color of the item", NORMAL_COLOR);
			itemColor->hideInEditor = true;
		}
	}
	else
	{
		if (itemColor != nullptr)
		{
			removeControllable(itemColor);
			itemColor = nullptr;
		}
	}
}

var BaseItem::getJSONData(bool includeNonOverriden)
{
	var data = ControllableContainer::getJSONData(includeNonOverriden);
	if (saveType) data.getDynamicObject()->setProperty("type", getTypeString());
	if (canHaveScripts) data.getDynamicObject()->setProperty("scripts", scriptManager->getJSONData());
	return data;
}

void BaseItem::loadJSONDataInternal(var data)
{
	ControllableContainer::loadJSONDataInternal(data);
	loadJSONDataItemInternal(data);
	if (canHaveScripts) scriptManager->loadJSONData(data.getProperty("scripts", var()));
}

void BaseItem::getRemoteControlDataInternal(var& data)
{
	ControllableContainer::getRemoteControlDataInternal(data);
	data.getDynamicObject()->setProperty("TYPE", getTypeString());
}

InspectableEditor* BaseItem::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new BaseItemEditor(this, isRoot);
}

BaseItemMinimalUI* BaseItem::createUI()
{
	return new BaseItemMinimalUI(this);
}

var BaseItem::getTypeStringFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (BaseItem* b = getObjectFromJS<BaseItem>(a)) return b->getTypeString();
	return var();
}

String BaseItem::getScriptTargetString()
{
	return "[" + niceName + " : " + getTypeString() + "]";
}
