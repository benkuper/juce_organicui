/*
  ==============================================================================

    BaseItem.cpp
    Created: 28 Oct 2016 8:04:25pm
    Author:  bkupe

  ==============================================================================
*/

BaseItem::BaseItem(const String &name, bool _canBeDisabled, bool _canHaveScripts) :
	EnablingControllableContainer(name.isEmpty() ? getTypeString() : name, _canBeDisabled),
	canHaveScripts(_canHaveScripts),
	userCanRemove(true),
	userCanDuplicate(true),
	askConfirmationBeforeRemove(true),
	isSavable(true),
	saveType(true),
	canBeReorderedInEditor(true),
	itemDataType(""),
	isClearing(false)
{
	saveAndLoadName = true;
	nameCanBeChangedByUser = true;

	if (canHaveScripts)
	{
		scriptManager.reset(new ScriptManager(this));
		scriptManager->addBaseManagerListener(this);
		addChildControllableContainer(scriptManager.get());
	}

	//For UI
	miniMode = addBoolParameter("MiniMode", "Set the mini mode", false);
	miniMode->hideInOutliner = true;
	miniMode->hideInEditor = true;

	listUISize = addFloatParameter("ListSize", "Size in list", 0, 0, 5000);
	listUISize->hideInEditor = true;
	listUISize->hideInOutliner = true;

	viewUIPosition = addPoint2DParameter("ViewUIPosition", "Position the view");
	//viewUIPosition->setBounds(-100000, -100000, 100000, 100000);
	viewUIPosition->hideInEditor = true;
	viewUIPosition->hideInOutliner = true;

	viewUISize = addPoint2DParameter("ViewUISize", "Size in the view");
	//viewUISize->setBounds(30, 60, 10000, 10000);
	var defaultSize;
	defaultSize.append(200);
	defaultSize.append(200);
	viewUISize->defaultValue = defaultSize;
	viewUISize->resetValue();
	viewUISize->defaultValue = viewUISize->getValue();
	viewUISize->hideInEditor = true;
	viewUISize->hideInOutliner = true;

	isUILocked = addBoolParameter("Locked", "if checked, item is locked in UI", false);
	isUILocked->hideInEditor = true;

	scriptObject.setMethod("getType", BaseItem::getTypeStringFromScript);
}

BaseItem::~BaseItem()
{
}

void BaseItem::clearItem() 
{
	isClearing = true;
	if(canHaveScripts && scriptManager != nullptr) scriptManager->clear();
}

void BaseItem::duplicate()
{
	baseItemListeners.call(&BaseItemListener::askForDuplicateItem, this);
}

void BaseItem::copy()
{
	var data = getJSONData();
	data.getDynamicObject()->setProperty("itemType", itemDataType);
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


void BaseItem::setMovePositionReference(bool setOtherSelectedItems)
{
	setMovePositionReferenceInternal();

	if (setOtherSelectedItems)
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i == this) continue;
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
			if (i == this) continue;
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

		DBG("anchor diff to selected < " << anchorDiffToReference.toString() << " / anchorTIme " << anchorPosition.toString());
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
			i->addUndoableMoveAction(actions);
		}

		numItems = items.size();
	}

	UndoMaster::getInstance()->performActions("Move " + String(numItems) + " item"+String(numItems > 1?"s":""), actions);
}

void BaseItem::addUndoableMoveAction(Array<UndoableAction *> &arrayToAdd)
{
	arrayToAdd.add(viewUIPosition->setUndoablePoint(movePositionReference, viewUIPosition->getPoint(), true));
}

void BaseItem::onContainerParameterChanged(Parameter * p)
{
	if (canHaveScripts)
	{
		Array<var> args;
		args.add(p->getScriptObject());
		scriptManager->callFunctionOnAllItems("localParamChanged", args);
	}

	onContainerParameterChangedInternal(p);
}

void BaseItem::onContainerTriggerTriggered(Trigger * t)
{
	if (canHaveScripts)
	{
		Array<var> args;
		args.add(t->getScriptObject());
		scriptManager->callFunctionOnAllItems("localParamChanged", args);
	}
}

void BaseItem::onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
	onControllableFeedbackUpdateInternal(cc, c);
}

void BaseItem::itemAdded(Script* script)
{
	script->warningResolveInspectable = this;
}

var BaseItem::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	if(saveType) data.getDynamicObject()->setProperty("type", getTypeString());
	if (canHaveScripts) data.getDynamicObject()->setProperty("scripts", scriptManager->getJSONData());
	return data; 
}

void BaseItem::loadJSONDataInternal(var data)
{
	ControllableContainer::loadJSONDataInternal(data);
	loadJSONDataItemInternal(data);
	if (canHaveScripts) scriptManager->loadJSONData(data.getProperty("scripts",var()));
}

InspectableEditor * BaseItem::getEditor(bool isRoot)
{
	return new BaseItemEditor(this, isRoot);
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
