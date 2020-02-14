#include "BaseItem.h"
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
