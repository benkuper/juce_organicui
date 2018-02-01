#include "ControllableContainer.h"
/*
 ==============================================================================

 ControllableContainer.cpp
 Created: 8 Mar 2016 1:15:36pm
 Author:  bkupe

 ==============================================================================
 */

ControllableComparator ControllableContainer::comparator;

ControllableContainer::ControllableContainer(const String & niceName) :
	ScriptTarget("", this),
	hasCustomShortName(false),
	skipControllableNameInAddress(false),
	nameCanBeChangedByUser(false),
	isTargettable(true),
	hideInEditor(false),
	canInspectChildContainers(true),
	editorIsCollapsed(false),
	editorCanBeCollapsed(true),
	hideEditorHeader(false),
	customGetEditorFunc(nullptr),
	saveAndLoadRecursiveData(false),
	saveAndLoadName(false),
	includeTriggersInSaveLoad(false),
    includeInScriptObject(true),
    parentContainer(nullptr),
    queuedNotifier(500) //what to put in max size ??
						//500 seems ok on my computer, but if too low, generates leaks when closing app while heavy use of async (like  parameter update from audio signal)
{
	setNiceName(niceName);

	//script
	scriptObject.setMethod("getChild", ControllableContainer::getChildFromScript);
}

ControllableContainer::~ControllableContainer()
{
	//controllables.clear();
	//DBG("CLEAR CONTROLLABLE CONTAINER");
	clear();
	masterReference.clear();
}


void ControllableContainer::clear() {

	controllables.clear();
	controllableContainers.clear();
	queuedNotifier.cancelPendingUpdate();
}


UndoableAction * ControllableContainer::addUndoableControllable(Controllable * c, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		addControllable(c);
		return nullptr;
	}

	UndoableAction * a = new AddControllableAction(this, c);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Add " + c->niceName, a);
	return a;
}

void ControllableContainer::addControllable(Controllable * c)
{
	if (c->type == Controllable::TRIGGER) addTriggerInternal((Trigger *)c);
	else addParameterInternal((Parameter *)c);
	c->addControllableListener(this);
}

void ControllableContainer::addParameter(Parameter * p)
{
	addControllable(p);
}

FloatParameter * ControllableContainer::addFloatParameter(const String & _niceName, const String & description, const float & initialValue, const float & minValue, const float & maxValue, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	FloatParameter * p = new FloatParameter(targetName, description, initialValue, minValue, maxValue, enabled);
	addControllable(p);
	return p;
}

IntParameter * ControllableContainer::addIntParameter(const String & _niceName, const String & _description, const int & initialValue, const int & minValue, const int & maxValue, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	IntParameter * p = new IntParameter(targetName, _description, initialValue, minValue, maxValue, enabled);
	addControllable(p);
	return p;
}

BoolParameter * ControllableContainer::addBoolParameter(const String & _niceName, const String & _description, const bool & value, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	BoolParameter * p = new BoolParameter(targetName, _description, value, enabled);
	addControllable(p);
	return p;
}

StringParameter * ControllableContainer::addStringParameter(const String & _niceName, const String & _description, const String &value, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	StringParameter * p = new StringParameter(targetName, _description, value, enabled);
	addControllable(p);
	return p;
}

EnumParameter * ControllableContainer::addEnumParameter(const String & _niceName, const String & _description, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	EnumParameter * p = new EnumParameter(targetName, _description, enabled);
	addControllable(p);
	return p;
}

Point2DParameter * ControllableContainer::addPoint2DParameter(const String & _niceName, const String & _description, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	Point2DParameter * p = new Point2DParameter(targetName, _description, enabled);
	addControllable(p);
	return p;
}

Point3DParameter * ControllableContainer::addPoint3DParameter(const String & _niceName, const String & _description, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	Point3DParameter * p = new Point3DParameter(targetName, _description, enabled);
	addControllable(p);
	return p;
}

TargetParameter * ControllableContainer::addTargetParameter(const String & _niceName, const String & _description, WeakReference<ControllableContainer> rootReference, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	TargetParameter * p = new TargetParameter(targetName, _description, "", rootReference, enabled);
	addControllable(p);
	return p;
}

Trigger * ControllableContainer::addTrigger(const String & _niceName, const String & _description, const bool & enabled)
{
	String targetName = getUniqueNameInContainer(_niceName);
	Trigger * t = new Trigger(targetName, _description, enabled);
	addControllable(t);
	return t;
}


void ControllableContainer::addTriggerInternal(Trigger * t)
{
	controllables.add(t);
	t->setParentContainer(this);
	t->addTriggerListener(this);

	controllableContainerListeners.call(&ControllableContainerListener::controllableAdded, t);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableAdded, this, t));
	notifyStructureChanged();
}

void ControllableContainer::addParameterInternal(Parameter * p)
{
	p->setParentContainer(this);
	controllables.add(p);
	p->addParameterListener(this);
	p->addAsyncParameterListener(this);
	controllableContainerListeners.call(&ControllableContainerListener::controllableAdded, p);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableAdded, this, p));
	notifyStructureChanged();
}

UndoableAction * ControllableContainer::removeUndoableControllable(Controllable * c, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		addControllable(c);
		return nullptr;
	}

	UndoableAction * a = new RemoveControllableAction(this, c);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Remove " + c->niceName, a);
	return a;
}

void ControllableContainer::removeControllable(Controllable * c)
{
	controllableContainerListeners.call(&ControllableContainerListener::controllableRemoved, c);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableRemoved, this, c));

	if (Parameter * p = dynamic_cast<Parameter*>(c)) {
		p->removeParameterListener(this);
		p->removeAsyncParameterListener(this);
	}

	c->removeControllableListener(this);

	controllables.removeObject(c);
	notifyStructureChanged();
}


void ControllableContainer::notifyStructureChanged() {

	liveScriptObjectIsDirty = true;

	controllableContainerListeners.call(&ControllableContainerListener::childStructureChanged, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ChildStructureChanged, this));

}

void ControllableContainer::newMessage(const Parameter::ParameterEvent &e) 
{
	if (e.type == Parameter::ParameterEvent::VALUE_CHANGED) {
		onContainerParameterChangedAsync(e.parameter, e.value);
	}
}
UndoableAction * ControllableContainer::setUndoableNiceName(const String & newNiceName, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		setNiceName(newNiceName);
		return nullptr;
	}

	UndoableAction * a = new ControllableContainerChangeNameAction(this, niceName, newNiceName);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Rename " + niceName, a);
	return a;	
}
void ControllableContainer::setNiceName(const String &_niceName) {
	if (niceName == _niceName) return;
	niceName = _niceName;
	if (!hasCustomShortName) setAutoShortName();
	onContainerNiceNameChanged();
}

void ControllableContainer::setCustomShortName(const String &_shortName) {
	shortName = _shortName;
	hasCustomShortName = true;
	scriptTargetName = shortName;
	updateChildrenControlAddress();
	onContainerShortNameChanged();
	controllableContainerListeners.call(&ControllableContainerListener::childAddressChanged, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ChildAddressChanged, this));

}

void ControllableContainer::setAutoShortName() {
	hasCustomShortName = false;
	shortName = StringUtil::toShortName(niceName,true);
	scriptTargetName = shortName;
	updateChildrenControlAddress();
	onContainerShortNameChanged();
	controllableContainerListeners.call(&ControllableContainerListener::childAddressChanged, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ChildAddressChanged, this));
}



Controllable * ControllableContainer::getControllableByName(const String & name, bool searchNiceNameToo)
{
	for (auto &c : controllables)
	{
		if (c->shortName == name || (searchNiceNameToo && c->niceName == name)) return c;
	}

	return nullptr;
}

void ControllableContainer::addChildControllableContainer(ControllableContainer * container)
{

	String targetName = getUniqueNameInContainer(container->niceName);
	container->setNiceName(targetName);

	controllableContainers.add(container);
	container->addControllableContainerListener(this);
	container->setParentContainer(this);
	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerAdded, container);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerAdded, this, container));

	notifyStructureChanged();
}

void ControllableContainer::removeChildControllableContainer(ControllableContainer * container)
{
	
	this->controllableContainers.removeAllInstancesOf(container);
	container->removeControllableContainerListener(this);
	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerRemoved, container);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerRemoved, this, container));

	notifyStructureChanged();
	container->setParentContainer(nullptr);
}


ControllableContainer * ControllableContainer::getControllableContainerByName(const String & name, bool searchNiceNameToo)
{
	for (auto &cc : controllableContainers)
	{
		if (cc.get() && (cc->shortName == name || (searchNiceNameToo && cc->niceName == name))) return cc;
	}

	return nullptr;

}

ControllableContainer * ControllableContainer::getControllableContainerForAddress(const String&  address, bool recursive, bool getNotExposed)
{
	StringArray addrArray;
	addrArray.addTokens(address, juce::StringRef("/"), juce::StringRef("\""));
	addrArray.remove(0);

	return getControllableContainerForAddress(addrArray, recursive, getNotExposed);
}

ControllableContainer * ControllableContainer::getControllableContainerForAddress(StringArray  addressSplit, bool recursive, bool getNotExposed)
{

	if (addressSplit.size() == 0) jassertfalse; // SHOULD NEVER BE THERE !

	bool isTargetFinal = addressSplit.size() == 1;

	if (isTargetFinal)
	{

		if (ControllableContainer * res = getControllableContainerByName(addressSplit[0]))   //get not exposed here here ?
			return res;

		//no found in direct children Container, maybe in a skip container ?
		for (auto &cc : controllableContainers)
		{
			if (cc->skipControllableNameInAddress)
			{
				if (ControllableContainer * res = cc->getControllableContainerForAddress(addressSplit, recursive, getNotExposed)) return res;
			}
		}
	} else //if recursive here ?
	{
		for (auto &cc : controllableContainers)
		{

			if (!cc->skipControllableNameInAddress)
			{
				if (cc->shortName == addressSplit[0])
				{
					addressSplit.remove(0);
					return cc->getControllableContainerForAddress(addressSplit, recursive, getNotExposed);
				}
			} else
			{
				ControllableContainer * tc = cc->getControllableContainerByName(addressSplit[0]);
				if (tc != nullptr)
				{
					addressSplit.remove(0);
					return tc->getControllableContainerForAddress(addressSplit, recursive, getNotExposed);
				}

			}
		}
	}

	return nullptr;

}

String ControllableContainer::getControlAddress(ControllableContainer * relativeTo) {

	StringArray addressArray;
	ControllableContainer * pc = this;
	while (pc != relativeTo && pc != nullptr)
	{
		if (!pc->skipControllableNameInAddress) addressArray.insert(0, pc->shortName);
		pc = pc->parentContainer;
	}
	if (addressArray.size() == 0)return "";
	else return "/" + addressArray.joinIntoString("/");
}

void ControllableContainer::orderControllablesAlphabetically()
{
	controllables.sort(ControllableContainer::comparator, true);
	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerReordered, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerReordered, this));

}

void ControllableContainer::setParentContainer(ControllableContainer * container)
{
	this->parentContainer = container;
	for (auto &c : controllables) c->updateControlAddress();
	for (auto &cc : controllableContainers) cc->updateChildrenControlAddress();

}

void ControllableContainer::updateChildrenControlAddress()
{
	for (auto &c : controllables) c->updateControlAddress();
	for (auto &cc : controllableContainers) cc->updateChildrenControlAddress();


}

Array<WeakReference<Controllable>> ControllableContainer::getAllControllables(bool recursive, bool getNotExposed)
{
	Array<WeakReference<Controllable>> result;
	for (auto &c : controllables)
	{
		if (getNotExposed || c->isControllableExposed) result.add(c);
	}

	if (recursive)
	{
		for (auto &cc : controllableContainers)
		{
			if (cc.wasObjectDeleted() || cc.get() == nullptr) continue;
			result.addArray(cc->getAllControllables(true, getNotExposed));
		}
	}

	return result;

}

Array<WeakReference<Parameter>> ControllableContainer::getAllParameters(bool recursive, bool getNotExposed)
{
	Array<WeakReference<Parameter>> result;

	for (auto &c : controllables)
	{
		if (c->type == Controllable::Type::TRIGGER) continue;
		if (getNotExposed || c->isControllableExposed) {
			if (Parameter * cc = dynamic_cast<Parameter*>(c)) {
				result.add(cc);
			}
		}
	}

	if (recursive)
	{
		for (auto &cc : controllableContainers) result.addArray(cc->getAllParameters(true, getNotExposed));
	}

	return result;
}



Controllable * ControllableContainer::getControllableForAddress(const String &address, bool recursive, bool getNotExposed)
{
	StringArray addrArray;
	addrArray.addTokens(address, juce::StringRef("/"), juce::StringRef("\""));
	addrArray.remove(0);

	return getControllableForAddress(addrArray, recursive, getNotExposed);
}

Controllable * ControllableContainer::getControllableForAddress(StringArray addressSplit, bool recursive, bool getNotExposed)
{
	if (addressSplit.size() == 0) jassertfalse; // SHOULD NEVER BE THERE !

	bool isTargetAControllable = addressSplit.size() == 1;

	if (isTargetAControllable)
	{
		//DBG("Check controllable Address : " + shortName);
		for (auto &c : controllables)
		{
			if (c->shortName == addressSplit[0])
			{
				//DBG(c->shortName);
				if (c->isControllableExposed || getNotExposed) return c;
				else return nullptr;
			}
		}

		//no found in direct children controllables, maybe in a skip container ?
		for (auto &cc : controllableContainers)
		{
			if (cc.wasObjectDeleted()) continue;
			if (cc->skipControllableNameInAddress)
			{
				Controllable * tc = cc->getControllableByName(addressSplit[0]);  //get not exposed here here
				if (tc != nullptr) return tc;
			}
		}
	} else  //if recursive here ?
	{
		for (auto &cc : controllableContainers)
		{
			if (cc.wasObjectDeleted()) continue;
			if (!cc->skipControllableNameInAddress)
			{
				if (cc->shortName == addressSplit[0])
				{
					addressSplit.remove(0);
					return cc->getControllableForAddress(addressSplit, recursive, getNotExposed);
				}
			} else
			{
				ControllableContainer * tc = cc->getControllableContainerByName(addressSplit[0]);
				if (tc != nullptr)
				{
					addressSplit.remove(0);
					return tc->getControllableForAddress(addressSplit, recursive, getNotExposed);
				}

			}
		}
	}

	return nullptr;
}

bool ControllableContainer::containsControllable(Controllable * c, int maxSearchLevels)
{
	if (c == nullptr) return false;

	ControllableContainer * pc = c->parentContainer;
	if (pc == nullptr) return false;
	int curLevel = 0;

	while (pc != nullptr)
	{
		if (pc == this) return true;
		curLevel++;
		if (maxSearchLevels >= 0 && curLevel > maxSearchLevels) return false;
		pc = pc->parentContainer;
	}

	return false;
}


void ControllableContainer::dispatchFeedback(Controllable * c)
{
	//    @ben removed else here to enable containerlistener call back of non root (proxies) is it overkill?
	if (parentContainer != nullptr) { parentContainer->dispatchFeedback(c); }
	controllableContainerListeners.call(&ControllableContainerListener::controllableFeedbackUpdate, this, c);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableFeedbackUpdate, this, c));
}


void ControllableContainer::parameterValueChanged(Parameter * p)
{
	if (p->parentContainer == this)
	{
		onContainerParameterChanged(p);
		if (p->isControllableExposed) dispatchFeedback(p);
	} else
	{
		onExternalParameterChanged(p);
	}

}


void ControllableContainer::triggerTriggered(Trigger * t)
{
	if (t->parentContainer == this) onContainerTriggerTriggered(t);
	else onExternalTriggerTriggered(t);


	if (t->isControllableExposed) dispatchFeedback(t);
}

void ControllableContainer::controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{ 
	onControllableFeedbackUpdate(cc, c); //This is the function to override from child classes
}

void ControllableContainer::askForRemoveControllable(Controllable * c, bool addToUndo)
{
	if (addToUndo) removeUndoableControllable(c);
	else removeControllable(c);
}


var ControllableContainer::getJSONData()
{
	var data(new DynamicObject());

	var paramsData;

	
	Array<WeakReference<Controllable>> cont = ControllableContainer::getAllControllables(saveAndLoadRecursiveData, true);

	for (auto &wc : cont) {
		if (wc->type == Controllable::TRIGGER && !includeTriggersInSaveLoad) continue;
		if (wc.wasObjectDeleted()) continue;
		if (!wc->isSavable) continue;
		Parameter * p = dynamic_cast<Parameter *>(wc.get());
		if (p != nullptr && p->saveValueOnly && !p->isOverriden && !p->forceSaveValue && p->controlMode == Parameter::ControlMode::MANUAL) continue; //do not save parameters that have not changed. it should light up the file. But save custom-made parameters even if there not overriden !
		paramsData.append(wc->getJSONData(this));
	}

	//data.getDynamicObject()->setProperty("uid", uid.toString());
	if(paramsData.size() > 0) data.getDynamicObject()->setProperty("parameters", paramsData);
	
	if (saveAndLoadName)
	{
		data.getDynamicObject()->setProperty("niceName", niceName);
		if (hasCustomShortName) data.getDynamicObject()->setProperty("shortName", shortName);
	}

	if (editorIsCollapsed) data.getDynamicObject()->setProperty("editorIsCollapsed", true); //only set if true to avoid too much data

	return data;
}

void ControllableContainer::loadJSONData(var data, bool createIfNotThere)
{
	if (data.isVoid()) return;
	if (data.getDynamicObject() == nullptr) return;

	//if (data.getDynamicObject()->hasProperty("uid")) uid = data.getDynamicObject()->getProperty("uid");
	if (data.getDynamicObject()->hasProperty("niceName")) setNiceName(data.getDynamicObject()->getProperty("niceName"));
	if (data.getDynamicObject()->hasProperty("shortName")) setCustomShortName(data.getDynamicObject()->getProperty("shortName"));
	if (data.getDynamicObject()->hasProperty("editorIsCollapsed")) editorIsCollapsed = data.getDynamicObject()->getProperty("editorIsCollapsed");

	Array<var> * paramsData = data.getDynamicObject()->getProperty("parameters").getArray();

	if (paramsData != nullptr)
	{
		for (var &pData : *paramsData)
		{
			DynamicObject * o = pData.getDynamicObject();
			String pControlAddress = o->getProperty("controlAddress");

			Controllable * c = getControllableForAddress(pControlAddress, saveAndLoadRecursiveData, true);

			if (c != nullptr)
			{
				if (Parameter * p = dynamic_cast<Parameter*>(c)) {
					if (p->isSavable) p->loadJSONData(pData.getDynamicObject());
				}

			} else if (!saveAndLoadRecursiveData && createIfNotThere)
			{
				c = ControllableFactory::getInstance()->createControllable(o->getProperty("type"));
				if (c != nullptr)
				{
					c->saveValueOnly = false; //auto set here because it will likely need that if it has been created from data
					c->loadJSONData(pData);
					addControllable(c);
				}
			}
		}
	}

	loadJSONDataInternal(data);
}

void ControllableContainer::childStructureChanged(ControllableContainer * cc)
{
	notifyStructureChanged();
}

void ControllableContainer::childAddressChanged(ControllableContainer * cc)
{
	notifyStructureChanged();
}

void ControllableContainer::parameterRangeChanged(Parameter * p)
{
	notifyStructureChanged();
}

String ControllableContainer::getUniqueNameInContainer(const String & sourceName, int suffix)
{
	String resultName = sourceName;
	if (suffix > 0)
	{
		StringArray sa;
		sa.addTokens(resultName, false);
		if (sa.size() > 1 && (sa[sa.size() - 1].getIntValue() != 0 || sa[sa.size() - 1].containsOnly("0")))
		{
			int num = sa[sa.size() - 1].getIntValue() + suffix;
			sa.remove(sa.size() - 1);
			sa.add(String(num));
			resultName = sa.joinIntoString(" ");
		} else
		{
			resultName += " " + String(suffix);
		}
	}

	if (getControllableByName(resultName, true) != nullptr)
	{
		return getUniqueNameInContainer(sourceName, suffix + 1);
	}

	if (getControllableContainerByName(resultName, true) != nullptr)
	{
		return getUniqueNameInContainer(sourceName, suffix + 1);
	}

	return resultName;
}

void ControllableContainer::updateLiveScriptObjectInternal(DynamicObject * parent) 
{	
	ScriptTarget::updateLiveScriptObjectInternal(parent);
	

	bool transferToParent = parent != nullptr;
	
	for (auto &cc : controllableContainers)
	{
		if (cc == nullptr || cc.wasObjectDeleted()) continue;

		if (!cc->includeInScriptObject) continue;
		if (cc->skipControllableNameInAddress)
		{
			cc->updateLiveScriptObject(transferToParent?parent:(DynamicObject *)liveScriptObject);
		}else
		{
			if (transferToParent) parent->setProperty(cc->shortName, cc->getScriptObject());
			else liveScriptObject->setProperty(cc->shortName, cc->getScriptObject());
		}

	}

	for (auto &c : controllables)
	{
		if (!c->includeInScriptObject) continue;
		if(transferToParent) parent->setProperty(c->shortName,c->getScriptObject());
		else liveScriptObject->setProperty(c->shortName, c->getScriptObject());
	}
	
	if (!(skipControllableNameInAddress && parent != nullptr))
	{
		liveScriptObject->setProperty("name", shortName);
		liveScriptObject->setProperty("niceName", niceName);
	}
}

var ControllableContainer::getChildFromScript(const var::NativeFunctionArgs & a)
{
	if (a.numArguments == 0) return var();
	ControllableContainer * m = getObjectFromJS<ControllableContainer>(a);
	if (m == nullptr) return var();
	Controllable * c = m->getControllableByName(a.arguments[0].toString());
	if (c == nullptr) return  var();
	return c->getScriptObject();
}


InspectableEditor * ControllableContainer::getEditor(bool isRoot)
{
	if (customGetEditorFunc != nullptr) return customGetEditorFunc(this, isRoot);
	return new GenericControllableContainerEditor(this, isRoot);
}

EnablingControllableContainer::EnablingControllableContainer(const String & n, bool _canBeDisabled) :
	ControllableContainer(n),
	canBeDisabled(_canBeDisabled)
{
	if (canBeDisabled)
	{
		enabled = addBoolParameter("Enabled", "Activate OSC Input for this module", true);
		enabled->hideInEditor = true;
	}
	
}

InspectableEditor * EnablingControllableContainer::getEditor(bool isRoot)
{
	if (customGetEditorFunc != nullptr) return customGetEditorFunc(this, isRoot);
	return new EnablingControllableContainerEditor(this, isRoot);
}

ControllableContainer * ControllableContainer::ControllableContainerAction::getControllableContainer()
{
	if (containerRef != nullptr && !containerRef.wasObjectDeleted()) return containerRef.get();
	else
	{
		ControllableContainer * cc = Engine::mainEngine->getControllableContainerForAddress(controlAddress, true);
		return cc;
	}
}


bool ControllableContainer::ControllableContainerChangeNameAction::perform()
{
	ControllableContainer * cc = getControllableContainer();
	if (cc != nullptr)
	{
		cc->setNiceName(newName);
		return true;
	}
	return false;
}

bool ControllableContainer::ControllableContainerChangeNameAction::undo()
{
	ControllableContainer * cc = getControllableContainer();
	if (cc != nullptr)
	{
		cc->setNiceName(oldName);
		return true;
	}
	return false;
}

ControllableContainer::ControllableContainerControllableAction::ControllableContainerControllableAction(ControllableContainer * cc, Controllable * c) :
	ControllableContainerAction(cc),
	cRef(c)
{
	if (c != nullptr)
	{
		cShortName = c->shortName;
		data = c->getJSONData();
		cType = c->getTypeString();
	}
}

Controllable * ControllableContainer::ControllableContainerControllableAction::getItem()
{
	if (cRef != nullptr && !cRef.wasObjectDeleted()) return dynamic_cast<Controllable *>(cRef.get());
	else
	{
		ControllableContainer * cc = this->getControllableContainer();
		if (cc != nullptr) return cc->getControllableByName(cShortName);
	}

	return nullptr;
}

bool ControllableContainer::AddControllableAction::perform()
{
	ControllableContainer * cc = this->getControllableContainer();
	if (cc == nullptr)
	{
		return false;
	}

	Controllable * c = this->getItem();
	if (c != nullptr)
	{
		cc->addControllable(c);
	} else
	{
		c = ControllableFactory::createControllable(cType);
	}

	if (c == nullptr) return false;

	this->cShortName = c->shortName;
	return true;
}

bool ControllableContainer::AddControllableAction::undo()
{
	Controllable * c = this->getItem();
	if (c == nullptr) return false;
	data = c->getJSONData();
	ControllableContainer * cc = getControllableContainer();
	if (cc != nullptr)
	{
		cc->removeControllable(c);
		cRef = nullptr;
	}
	return true;
}

ControllableContainer::RemoveControllableAction::RemoveControllableAction(ControllableContainer * cc, Controllable * c) :
	ControllableContainerControllableAction(cc, c)
{
}

bool ControllableContainer::RemoveControllableAction::perform()
{
	Controllable * c = this->getItem();

	if (c == nullptr) return false;
	getControllableContainer()->removeControllable(c);
	cRef = nullptr;
	return true;
}

bool ControllableContainer::RemoveControllableAction::undo()
{
	ControllableContainer * cc = getControllableContainer();
	if (cc == nullptr) return false;
	Controllable * c = ControllableFactory::createControllable(cType);
	if (c != nullptr)
	{
		c->loadJSONData(data); 
		cc->addControllable(c);
		cRef = c;
	}
	return true;
}
