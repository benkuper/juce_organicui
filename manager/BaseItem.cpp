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
	askConfirmationBeforeRemove(true),
	itemDataType("")
{
	saveAndLoadName = true;
	nameCanBeChangedByUser = true;

	if (canHaveScripts)
	{
		scriptManager = new ScriptManager(this);
		addChildControllableContainer(scriptManager);
	}

	//For UI
	miniMode = addBoolParameter("Mini Mode", "Set the mini mode", false);
	miniMode->hideInOutliner = true;
	miniMode->hideInEditor = true;
	miniMode->isTargettable = false;

	listUISize = addFloatParameter("List Size", "Size in list", 0, 0, 500);
	listUISize->hideInEditor = true;
	listUISize->hideInOutliner = true;
	listUISize->isTargettable = false;

	viewUIPosition = addPoint2DParameter("ViewUIPosition", "Position the view");
	viewUIPosition->setBounds(-100000, -100000, 100000, 100000);
	viewUIPosition->hideInEditor = true;
	viewUIPosition->hideInOutliner = true;
	viewUIPosition->isTargettable = false;

	viewUISize = addPoint2DParameter("Size", "Size in the view");
	viewUISize->setBounds(30, 60, 10000, 10000);
	viewUISize->setPoint(200, 200);
	viewUISize->defaultValue = viewUISize->value;
	viewUISize->hideInEditor = true;
	viewUISize->hideInOutliner = true;
	viewUISize->isTargettable = false;
}

BaseItem::~BaseItem()
{
	clear();
}

void BaseItem::duplicate()
{
	baseItemListeners.call(&BaseItem::Listener::askForDuplicateItem, this);
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
	baseItemListeners.call(&BaseItem::Listener::askForPaste);
	return true;
}

void BaseItem::remove()
{
	baseItemListeners.call(&BaseItem::Listener::askForRemoveBaseItem, this);
}


void BaseItem::onContainerParameterChanged(Parameter * p)
{
	Array<var> args;
	args.add(p->getScriptObject());
	if (canHaveScripts) scriptManager->callFunctionOnAllItems("localParamChanged", args);

	onContainerParameterChangedInternal(p);
}

void BaseItem::onContainerTriggerTriggered(Trigger * t)
{
	Array<var> args;
	args.add(t->getScriptObject());
	if (canHaveScripts) scriptManager->callFunctionOnAllItems("localParamChanged", args);
}

void BaseItem::onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
	onControllableFeedbackUpdateInternal(cc, c);
}

var BaseItem::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	data.getDynamicObject()->setProperty("type", getTypeString());
	if (canHaveScripts) data.getDynamicObject()->setProperty("scripts", scriptManager->getJSONData());
	return data; 
}

void BaseItem::loadJSONDataInternal(var data)
{
	ControllableContainer::loadJSONDataInternal(data);
	if (canHaveScripts) scriptManager->loadJSONData(data.getProperty("scripts",var()));
}

InspectableEditor * BaseItem::getEditor(bool isRoot)
{
	return new BaseItemEditor(this, isRoot);
}
