/*
 ==============================================================================

 ControllableContainer.cpp
 Created: 8 Mar 2016 1:15:36pm
 Author:  bkupe

 ==============================================================================
 */

ControllableComparator ControllableContainer::comparator;

ControllableContainer::ControllableContainer(const String& niceName) :
	ScriptTarget("", this, "Container"),
	hasCustomShortName(false),
	allowSameChildrenNiceNames(false),
	nameCanBeChangedByUser(false),
	canInspectChildContainers(true),
	editorIsCollapsed(false),
	editorCanBeCollapsed(true),
	hideEditorHeader(false),
	skipLabelInTarget(false),
	userCanAddControllables(false),
	isRemovableByUser(false),
	customUserCreateControllableFunc(nullptr),
	customGetEditorFunc(nullptr),
	saveAndLoadRecursiveData(false),
	saveAndLoadName(false),
	includeInRecursiveSave(true),
	includeTriggersInSaveLoad(false),
	isCurrentlyLoadingData(false),
	notifyStructureChangeWhenLoadingData(true),
	canBeCopiedAndPasted(false),
	includeInScriptObject(true),
	customControllableComparator(nullptr),
	parentContainer(nullptr),
	queuedNotifier(500) //what to put in max size ??
						//500 seems ok on my computer, but if too low, generates leaks when closing app while heavy use of async (like  parameter update from audio signal)
{
	setNiceName(niceName);

	//script
	scriptObject.setMethod("getChild", ControllableContainer::getChildFromScript);
	scriptObject.setMethod("getParent", ControllableContainer::getParentFromScript);
	scriptObject.setMethod("setName", ControllableContainer::setNameFromScript);
	scriptObject.setMethod("setCollapsed", ControllableContainer::setCollapsedFromScript);
	
	scriptObject.setMethod("addTrigger", ControllableContainer::addTriggerFromScript);
	scriptObject.setMethod("addBoolParameter", ControllableContainer::addBoolParameterFromScript);
	scriptObject.setMethod("addIntParameter", ControllableContainer::addIntParameterFromScript);
	scriptObject.setMethod("addFloatParameter", ControllableContainer::addFloatParameterFromScript);
	scriptObject.setMethod("addStringParameter", ControllableContainer::addStringParameterFromScript);
	scriptObject.setMethod("addEnumParameter", ControllableContainer::addEnumParameterFromScript);
	scriptObject.setMethod("addTargetParameter", ControllableContainer::addTargetParameterFromScript);
	scriptObject.setMethod("addPoint2DParameter", ControllableContainer::addPoint2DParameterFromScript);
	scriptObject.setMethod("addPoint3DParameter", ControllableContainer::addPoint3DParameterFromScript);
	scriptObject.setMethod("addColorParameter", ControllableContainer::addColorParameterFromScript);
	scriptObject.setMethod("addFileParameter", ControllableContainer::addFileParameterFromScript);
	scriptObject.setMethod("addContainer", ControllableContainer::addContainerFromScript);
	scriptObject.setMethod("removeContainer", ControllableContainer::removeContainerFromScript);
	scriptObject.setMethod("removeParameter", ControllableContainer::removeControllableFromScript);

	scriptObject.setMethod("getControlAddress", ControllableContainer::getControlAddressFromScript);
	scriptObject.setMethod("getScriptControlAdress", ControllableContainer::getScriptControlAddressFromScript);
}

ControllableContainer::~ControllableContainer()
{
	//controllables.clear();
	//DBG("CLEAR CONTROLLABLE CONTAINER");
	clear();
	masterReference.clear();
}


void ControllableContainer::clear() {

	queuedNotifier.cancelPendingUpdate();

	controllables.clear();
	controllableContainers.clear();
	ownedContainers.clear();
}


UndoableAction* ControllableContainer::addUndoableControllable(Controllable* c, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		addControllable(c);
		return nullptr;
	}

	UndoableAction* a = new AddControllableAction(this, c);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Add " + c->niceName, a);
	return a;
}

void ControllableContainer::addControllable(Controllable* c)
{
	if (c == nullptr)
	{
		DBG("Controllable is null !");
		return;
	}

	if (c->type == Controllable::TRIGGER) addTriggerInternal((Trigger*)c);
	else addParameterInternal((Parameter*)c);

	c->addControllableListener(this);
	c->addAsyncWarningTargetListener(this);
	c->warningResolveInspectable = this;
}

void ControllableContainer::addParameter(Parameter* p)
{
	addControllable(p);
}

FloatParameter* ControllableContainer::addFloatParameter(const String& _niceName, const String& description, const float& initialValue, const float& minValue, const float& maxValue, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	FloatParameter* p = new FloatParameter(targetName, description, initialValue, minValue, maxValue, enabled);
	addControllable(p);
	return p;
}

IntParameter* ControllableContainer::addIntParameter(const String& _niceName, const String& _description, const int& initialValue, const int& minValue, const int& maxValue, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	IntParameter* p = new IntParameter(targetName, _description, initialValue, minValue, maxValue, enabled);
	addControllable(p);
	return p;
}

BoolParameter* ControllableContainer::addBoolParameter(const String& _niceName, const String& _description, const bool& value, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	BoolParameter* p = new BoolParameter(targetName, _description, value, enabled);
	addControllable(p);
	return p;
}

StringParameter* ControllableContainer::addStringParameter(const String& _niceName, const String& _description, const String& value, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	StringParameter* p = new StringParameter(targetName, _description, value, enabled);
	addControllable(p);
	return p;
}

EnumParameter* ControllableContainer::addEnumParameter(const String& _niceName, const String& _description, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	EnumParameter* p = new EnumParameter(targetName, _description, enabled);
	addControllable(p);
	return p;
}

Point2DParameter* ControllableContainer::addPoint2DParameter(const String& _niceName, const String& _description, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	Point2DParameter* p = new Point2DParameter(targetName, _description, enabled);
	addControllable(p);
	return p;
}

Point3DParameter* ControllableContainer::addPoint3DParameter(const String& _niceName, const String& _description, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	Point3DParameter* p = new Point3DParameter(targetName, _description, enabled);
	addControllable(p);
	return p;
}

ColorParameter* ControllableContainer::addColorParameter(const String& _niceName, const String& _description, const Colour& initialColor, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	ColorParameter* p = new ColorParameter(targetName, _description, initialColor, enabled);
	addControllable(p);
	return p;
}

TargetParameter* ControllableContainer::addTargetParameter(const String& _niceName, const String& _description, WeakReference<ControllableContainer> rootReference, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	TargetParameter* p = new TargetParameter(targetName, _description, "", rootReference, enabled);
	addControllable(p);
	return p;
}

FileParameter* ControllableContainer::addFileParameter(const String& _niceName, const String& _description, const String& _initialValue)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	FileParameter* p = new FileParameter(targetName, _description, _initialValue);
	addControllable(p);
	return p;
}

Trigger* ControllableContainer::addTrigger(const String& _niceName, const String& _description, const bool& enabled)
{
	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? _niceName : getUniqueNameInContainer(_niceName);
	Trigger* t = new Trigger(targetName, _description, enabled);
	addControllable(t);
	return t;
}


void ControllableContainer::addTriggerInternal(Trigger* t)
{
	controllables.add(t);

	t->setParentContainer(this);
	t->addTriggerListener(this);
	onControllableAdded(t);
	controllableContainerListeners.call(&ControllableContainerListener::controllableAdded, t);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableAdded, this, t));
	notifyStructureChanged();
}

void ControllableContainer::addParameterInternal(Parameter* p)
{
	p->setParentContainer(this);

	controllables.add(p);

	p->addParameterListener(this);
	p->addAsyncParameterListener(this);
	onControllableAdded(p);
	controllableContainerListeners.call(&ControllableContainerListener::controllableAdded, p);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableAdded, this, p));
	notifyStructureChanged();
}

UndoableAction* ControllableContainer::removeUndoableControllable(Controllable* c, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		addControllable(c);
		return nullptr;
	}

	UndoableAction* a = new RemoveControllableAction(this, c);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Remove " + c->niceName, a);
	return a;
}

void ControllableContainer::removeControllable(WeakReference<Controllable> c)
{
	if (c == nullptr || c.wasObjectDeleted())
	{
		DBG("Remove controllable but ref was deleted");
		return;
	}

	controllableContainerListeners.call(&ControllableContainerListener::controllableRemoved, c);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableRemoved, this, c));
	
	if (c != nullptr)
	{
		c->removeAsyncWarningTargetListener(this);
		c->removeControllableListener(this);

		if (Parameter* p = dynamic_cast<Parameter*>(c.get()))
		{
			p->removeParameterListener(this);
			p->removeAsyncParameterListener(this);
		}
	}
	

	onControllableRemoved(c);

	controllables.removeObject(c);

	notifyStructureChanged();
}


void ControllableContainer::notifyStructureChanged()
{
	if (isCurrentlyLoadingData && !notifyStructureChangeWhenLoadingData) return;

	liveScriptObjectIsDirty = true;

	controllableContainerListeners.call(&ControllableContainerListener::childStructureChanged, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ChildStructureChanged, this));

}

void ControllableContainer::newMessage(const Parameter::ParameterEvent& e)
{
	if (e.type == Parameter::ParameterEvent::VALUE_CHANGED) {
		onContainerParameterChangedAsync(e.parameter, e.value);
	}
}
void ControllableContainer::newMessage(const WarningTarget::WarningTargetEvent& e)
{
	switch (e.type)
	{
	case WarningTarget::WarningTargetEvent::WARNING_CHANGED:
		if(!e.target.wasObjectDeleted()) warningChanged(e.target);
		break;
	}
}

UndoableAction* ControllableContainer::setUndoableNiceName(const String& newNiceName, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		setNiceName(newNiceName);
		return nullptr;
	}

	UndoableAction* a = new ControllableContainerChangeNameAction(this, niceName, newNiceName);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Rename " + niceName, a);
	return a;
}
void ControllableContainer::setNiceName(const String& _niceName) {
	if (niceName == _niceName) return;
	niceName = _niceName;
	if (!hasCustomShortName) setAutoShortName();
	liveScriptObjectIsDirty = true;
	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerNameChanged, this);
	onContainerNiceNameChanged();
}

void ControllableContainer::setCustomShortName(const String& _shortName) {
	shortName = _shortName;
	hasCustomShortName = true;
	scriptTargetName = shortName;
	liveScriptObjectIsDirty = true;
	updateChildrenControlAddress();
	onContainerShortNameChanged();
	controllableContainerListeners.call(&ControllableContainerListener::childAddressChanged, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ChildAddressChanged, this));

}

void ControllableContainer::setAutoShortName() {
	hasCustomShortName = false;
	shortName = StringUtil::toShortName(niceName, true);
	scriptTargetName = shortName;
	liveScriptObjectIsDirty = true;
	updateChildrenControlAddress();
	onContainerShortNameChanged();
	controllableContainerListeners.call(&ControllableContainerListener::childAddressChanged, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ChildAddressChanged, this));
}



Controllable* ControllableContainer::getControllableByName(const String& name, bool searchNiceNameToo, bool searchLowerCaseToo)
{
	
	for (auto& c : controllables)
	{
		if (c->shortName == name || (searchNiceNameToo && c->niceName == name) || (searchLowerCaseToo && c->shortName.toLowerCase() == name.toLowerCase())) return c;
	}
	

	return nullptr;
}

Parameter* ControllableContainer::getParameterByName(const String& name, bool searchNiceNameToo, bool searchLowerCaseToo)
{
	return dynamic_cast<Parameter*>(getControllableByName(name, searchNiceNameToo, searchLowerCaseToo));
}

void ControllableContainer::addChildControllableContainer(ControllableContainer* container, bool owned, int index, bool notify)
{

	String targetName = (Engine::mainEngine == nullptr || Engine::mainEngine->isLoadingFile) ? container->niceName : getUniqueNameInContainer(container->niceName);
	container->setNiceName(targetName);

	controllableContainers.insert(index, container);
	if (owned) ownedContainers.add(container);

	container->addControllableContainerListener(this);
	container->addAsyncWarningTargetListener(this);
	container->setParentContainer(this);

	if (Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile)
	{
		if (container->getWarningMessage().isNotEmpty()) warningChanged(container);
	}

	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerAdded, container);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerAdded, this, container));

	if (notify) notifyStructureChanged();
}

void ControllableContainer::addChildControllableContainers(Array<ControllableContainer*> containers, bool owned, int index, bool notify)
{
	int i = index;

	for (auto& c : containers)
	{
		addChildControllableContainer(c, owned, i, false);
		++i;
	}

	if (notify) notifyStructureChanged();
}

void ControllableContainer::removeChildControllableContainer(ControllableContainer* container)
{
	this->controllableContainers.removeAllInstancesOf(container);

	container->removeControllableContainerListener(this);
	container->removeAsyncWarningTargetListener(this);

	if (Engine::mainEngine != nullptr && !Engine::mainEngine->isClearing)
	{
		warningChanged(container);
	}

	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerRemoved, container);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerRemoved, this, container));

	notifyStructureChanged();
	container->setParentContainer(nullptr);
	if (ownedContainers.contains(container)) ownedContainers.removeObject(container);
}


ControllableContainer* ControllableContainer::getControllableContainerByName(const String& name, bool searchNiceNameToo, bool searchLowerCaseToo)
{
	controllableContainers.getLock().enter();
	ControllableContainer* result = nullptr;
	for (auto& cc : controllableContainers)
	{
		if (!cc.wasObjectDeleted() && cc != nullptr && (
			cc->shortName == name ||
			(searchNiceNameToo && cc->niceName == name) ||
			(searchLowerCaseToo && cc->shortName.toLowerCase() == name.toLowerCase())
			))
		{
			result = cc;
			break;
		}

	}
	controllableContainers.getLock().exit();


	return result;

}

ControllableContainer* ControllableContainer::getControllableContainerForAddress(const String& address, bool recursive, bool getNotExposed)
{
	StringArray addrArray;
	addrArray.addTokens(address, juce::StringRef("/"), juce::StringRef("\""));
	addrArray.remove(0);

	return getControllableContainerForAddress(addrArray, recursive, getNotExposed);
}

ControllableContainer* ControllableContainer::getControllableContainerForAddress(StringArray  addressSplit, bool recursive, bool getNotExposed)
{

	if (addressSplit.size() == 0) jassertfalse; // SHOULD NEVER BE THERE !

	bool isTargetFinal = addressSplit.size() == 1;

	if (isTargetFinal)
	{

		if (ControllableContainer* res = getControllableContainerByName(addressSplit[0], true))   //get not exposed here here ?
			return res;

	}
	else //if recursive here ?
	{
		ControllableContainer* result = nullptr;

		controllableContainers.getLock().enter();
		for (auto& cc : controllableContainers)
		{
			if (cc == nullptr || cc.wasObjectDeleted()) continue;

			if (cc->shortName == addressSplit[0])
			{
				addressSplit.remove(0);
				result = cc->getControllableContainerForAddress(addressSplit, recursive, getNotExposed);
			}

			if (result != nullptr) break;
		}
		controllableContainers.getLock().exit();

		if (result) return result;

	}

	return nullptr;

}

String ControllableContainer::getControlAddress(ControllableContainer* relativeTo) {

	StringArray addressArray;
	ControllableContainer* pc = this;
	while (pc != relativeTo && pc != nullptr && pc != Engine::mainEngine)
	{
		addressArray.insert(0, pc->shortName);
		pc = pc->parentContainer;
	}
	if (addressArray.size() == 0)return "";
	else return "/" + addressArray.joinIntoString("/");
}

void ControllableContainer::sortControllables()
{
	controllables.sort(customControllableComparator != nullptr?*customControllableComparator:ControllableContainer::comparator, true);

	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerReordered, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerReordered, this));

}

void ControllableContainer::setParentContainer(ControllableContainer* container)
{
	this->parentContainer = container;

	
	for (auto& c : controllables) if (c != nullptr) c->updateControlAddress();
	

	
	for (auto& cc : controllableContainers) if (!cc.wasObjectDeleted()) cc->updateChildrenControlAddress();
	

}

void ControllableContainer::updateChildrenControlAddress()
{
	
	for (auto& c : controllables)
	{
		if (c == nullptr)
		{
			jassertfalse; //should not be here
			continue;
		}
		c->updateControlAddress();
	}
	


	
	for (auto& cc : controllableContainers)
	{
		if (cc == nullptr)
		{
			jassertfalse; //should not be here
			continue;
		}
		cc->updateChildrenControlAddress();
	}
	

}

Array<WeakReference<Controllable>> ControllableContainer::getAllControllables(bool recursive, bool getNotExposed)
{
	Array<WeakReference<Controllable>> result;
	
	for (auto& c : controllables)
	{
		if (getNotExposed || c->isControllableExposed) result.add(c);
	}
	

	
	if (recursive)
	{
		for (auto& cc : controllableContainers)
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

	
	for (auto& c : controllables)
	{
		if (c->type == Controllable::Type::TRIGGER) continue;
		if (getNotExposed || c->isControllableExposed) {
			if (Parameter* cc = dynamic_cast<Parameter*>(c)) {
				result.add(cc);
			}
		}
	}
	

	if (recursive)
	{
		
		for (auto& cc : controllableContainers) result.addArray(cc->getAllParameters(true, getNotExposed));
		
	}

	return result;
}

Array<WeakReference<ControllableContainer>> ControllableContainer::getAllContainers(bool recursive)
{
	Array<WeakReference<ControllableContainer>> result;

	
	for (auto& cc : controllableContainers)
	{
		if (cc.wasObjectDeleted() || cc.get() == nullptr) continue;
		result.add(cc);
		if (recursive) result.addArray(cc->getAllContainers(true));
	}
	
	return result;
}



Controllable* ControllableContainer::getControllableForAddress(const String& address, bool recursive, bool getNotExposed)
{
	StringArray addrArray;
	addrArray.addTokens(address.startsWith("/") ? address : "/" + address, juce::StringRef("/"), juce::StringRef("\""));
	addrArray.remove(0);

	return getControllableForAddress(addrArray, recursive, getNotExposed);
}

Controllable* ControllableContainer::getControllableForAddress(StringArray addressSplit, bool recursive, bool getNotExposed)
{
	if (addressSplit.size() == 0) jassertfalse; // SHOULD NEVER BE THERE !

	bool isTargetAControllable = addressSplit.size() == 1;

	if (isTargetAControllable)
	{
		Controllable* c = getControllableByName(addressSplit[0], false, true);
		if (c != nullptr)
		{
			if (c->isControllableExposed || getNotExposed) return c;
			else return nullptr;

		}

	}
	else  //if recursive here ?
	{
		ControllableContainer* cc = getControllableContainerByName(addressSplit[0], false, true);
		if (cc != nullptr)
		{
			addressSplit.remove(0);
			return cc->getControllableForAddress(addressSplit, recursive, getNotExposed);
		}

	}
	return nullptr;
}

bool ControllableContainer::containsControllable(Controllable* c, int maxSearchLevels)
{
	if (c == nullptr) return false;

	ControllableContainer* pc = c->parentContainer;
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


void ControllableContainer::dispatchFeedback(Controllable* c)
{
	//    @ben removed else here to enable containerlistener call back of non root (proxies) is it overkill?
	if (parentContainer != nullptr) { parentContainer->dispatchFeedback(c); }
	if (!c->isControllableExposed) return;

	controllableContainerListeners.call(&ControllableContainerListener::controllableFeedbackUpdate, this, c);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableFeedbackUpdate, this, c));
}

void ControllableContainer::dispatchState(Controllable* c)
{
	onControllableStateChanged(c);

	if (parentContainer != nullptr) { parentContainer->dispatchState(c); }
	if (!c->isControllableExposed) return;

	controllableContainerListeners.call(&ControllableContainerListener::controllableStateUpdate, this, c);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableStateUpdate, this, c));
}


void ControllableContainer::controllableStateChanged(Controllable* c)
{
	if (c->parentContainer == this) dispatchState(c);
}

void ControllableContainer::parameterValueChanged(Parameter* p)
{
	if (p->parentContainer == this)
	{
		onContainerParameterChanged(p);
		dispatchFeedback(p);
	}
	else
	{
		onExternalParameterValueChanged(p);
	}

}


void ControllableContainer::parameterRangeChanged(Parameter* p)
{
	if (p->parentContainer == this)
	{
		dispatchFeedback(p);
	}
	else
	{
		onExternalParameterRangeChanged(p);
	}
}


void ControllableContainer::triggerTriggered(Trigger* t)
{
	if (t->parentContainer == this) onContainerTriggerTriggered(t);
	else onExternalTriggerTriggered(t);


	if (t->isControllableExposed) dispatchFeedback(t);
}

void ControllableContainer::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	onControllableFeedbackUpdate(cc, c); //This is the function to override from child classes
}

void ControllableContainer::controllableNameChanged(Controllable* c)
{
	if (allowSameChildrenNiceNames)
	{
		if (!isNameTaken(c->niceName, true, c)) c->setAutoShortName();
		else if (isNameTaken(c->shortName, false, c)) c->setCustomShortName(getUniqueNameInContainer(c->shortName, false));
	}
	else
	{
		if(isNameTaken(c->niceName, true, c)) c->setNiceName(getUniqueNameInContainer(c->niceName, true));
	}

	notifyStructureChanged();
}

void ControllableContainer::askForRemoveControllable(Controllable* c, bool addToUndo)
{
	if (addToUndo) removeUndoableControllable(c);
	else removeControllable(c);
}

void ControllableContainer::warningChanged(WarningTarget* target)
{
	notifyWarningChanged();
	onWarningChanged(target);
	if (parentContainer != nullptr) parentContainer->warningChanged(target);
}

String ControllableContainer::getWarningMessage() const
{
	StringArray s;
	if (WarningTarget::getWarningMessage().isNotEmpty()) s.add(WarningTarget::getWarningMessage());

	if (controllables.getLock().tryEnter())
	{
		for (auto& c : controllables)
		{
			if (c == nullptr) continue;
			String cs = c->getWarningMessage();
			if (cs.isNotEmpty())
			{
				s.add(c->parentContainer->niceName + " > " + c->niceName + " : " + cs);
			}
		}
		controllables.getLock().exit();
	}
	

	controllableContainers.getLock().enter();
	for (auto& cc : controllableContainers)
	{
		if (cc.wasObjectDeleted()) continue;
		String cs = cc->getWarningMessage();
		if (cs.isNotEmpty())
		{
			s.add(cs);
		}
	}
	controllableContainers.getLock().exit();

	return s.joinIntoString("\n");
}

String ControllableContainer::getWarningTargetName() const
{
	return niceName;
}

var ControllableContainer::getJSONData()
{
	var data(new DynamicObject());

	var paramsData;


	Array<WeakReference<Controllable>> cont = getAllControllables(false, true);

	
	for (auto& wc : cont) {
		if (wc->type == Controllable::TRIGGER && !includeTriggersInSaveLoad) continue;
		if (wc.wasObjectDeleted()) continue;
		if (!wc->isSavable) continue;
		if (Parameter* p = dynamic_cast<Parameter*>(wc.get()))
		{
			if (p->controlMode == Parameter::ControlMode::MANUAL && !p->forceSaveValue)
			{
				if ((!p->userCanSetReadOnly && p->isControllableFeedbackOnly) || (!p->isOverriden && p->saveValueOnly)) continue; //do not save parameters that have not changed. it should light up the file. But save custom-made parameters even if there not overriden !
			}
		}
		paramsData.append(wc->getJSONData(this));
	}
	

	//data.getDynamicObject()->setProperty("uid", uid.toString());
	if (paramsData.size() > 0) data.getDynamicObject()->setProperty("parameters", paramsData);

	if (saveAndLoadName)
	{
		data.getDynamicObject()->setProperty("niceName", niceName);
		if (hasCustomShortName) data.getDynamicObject()->setProperty("shortName", shortName);
	}

	if (editorIsCollapsed) data.getDynamicObject()->setProperty("editorIsCollapsed", true); //only set if true to avoid too much data

	bool isOwned = (parentContainer != nullptr && parentContainer->ownedContainers.contains(this));
	if (isOwned)
	{
		if (isRemovableByUser) data.getDynamicObject()->setProperty("removable", true);
		if (includeTriggersInSaveLoad) data.getDynamicObject()->setProperty("includeTriggers", true);
	}

	if (saveAndLoadRecursiveData)
	{
		var containersData = new DynamicObject();
		
		//ownedContainers.getLock().enter();
		for (auto& cc : controllableContainers)
		{
			if (!cc->includeInRecursiveSave) continue;

			var ccData = cc->getJSONData();
			if (ownedContainers.contains(cc))
			{
				ccData.getDynamicObject()->setProperty("owned", true);
				if (!saveAndLoadName) ccData.getDynamicObject()->setProperty("niceName", cc->niceName);
			}

			containersData.getDynamicObject()->setProperty(cc->shortName, ccData);
		}
		//ownedContainers.getLock().exit();
		

		if (containersData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty("containers", containersData);
	}

	return data;
}

void ControllableContainer::loadJSONData(var data, bool createIfNotThere)
{
	if (data.isVoid()) return;
	if (data.getDynamicObject() == nullptr) return;

	isCurrentlyLoadingData = true;
	//if (data.getDynamicObject()->hasProperty("uid")) uid = data.getDynamicObject()->getProperty("uid");
	if (data.getDynamicObject()->hasProperty("niceName")) setNiceName(data.getDynamicObject()->getProperty("niceName"));
	if (data.getDynamicObject()->hasProperty("shortName")) setCustomShortName(data.getDynamicObject()->getProperty("shortName"));
	if (data.getDynamicObject()->hasProperty("editorIsCollapsed")) editorIsCollapsed = data.getDynamicObject()->getProperty("editorIsCollapsed");
	if (data.getDynamicObject()->hasProperty("removable")) isRemovableByUser = data.getDynamicObject()->getProperty("removable");
	if (data.getDynamicObject()->hasProperty("includeTriggers")) includeTriggersInSaveLoad = data.getDynamicObject()->getProperty("includeTriggers");

	Array<var>* paramsData = data.getDynamicObject()->getProperty("parameters").getArray();

	if (paramsData != nullptr)
	{
		for (var& pData : *paramsData)
		{
			DynamicObject* o = pData.getDynamicObject();
			String pControlAddress = o->getProperty("controlAddress");

			Controllable* c = getControllableForAddress(pControlAddress, false);

			if (c != nullptr)
			{
				if (Parameter* p = dynamic_cast<Parameter*>(c)) {
					if (p->isSavable) p->loadJSONData(pData.getDynamicObject());
				}

			}
			else if (createIfNotThere)
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

	if (/*saveAndLoadRecursiveData && */data.hasProperty("containers"))
	{
		saveAndLoadRecursiveData = true;
		NamedValueSet ccData = data.getProperty("containers", var()).getDynamicObject()->getProperties();
		for (auto& nv : ccData)
		{
			ControllableContainer* cc = getControllableContainerByName(nv.name.toString());
			if (cc == nullptr && (createIfNotThere || nv.value.getProperty("owned", false)))
			{
				cc = new ControllableContainer(nv.value.getProperty("niceName", nv.name.toString()));
				addChildControllableContainer(cc, true);
			}

			if (cc != nullptr) cc->loadJSONData(nv.value, true);
		}
	}

	loadJSONDataInternal(data);

	isCurrentlyLoadingData = false;
	controllableContainerListeners.call(&ControllableContainerListener::controllableContainerFinishedLoading, this);
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerFinishedLoading, this));

	afterLoadJSONDataInternal();
}

void ControllableContainer::controllableContainerNameChanged(ControllableContainer* cc)
{
	if (allowSameChildrenNiceNames)
	{
		if (!isNameTaken(cc->niceName, true, nullptr, cc)) cc->setAutoShortName();
		else if (isNameTaken(cc->shortName, false, nullptr, cc)) cc->setCustomShortName(getUniqueNameInContainer(cc->shortName, false));
	}
	else
	{
		if (isNameTaken(cc->niceName, true, nullptr, cc)) cc->setNiceName(getUniqueNameInContainer(cc->niceName, true));

	}

}

void ControllableContainer::childStructureChanged(ControllableContainer* cc)
{
	notifyStructureChanged();
}

void ControllableContainer::childAddressChanged(ControllableContainer* cc)
{
	notifyStructureChanged();
}


bool ControllableContainer::isNameTaken(const String& name, bool searchNiceName, Controllable* excludeC, ControllableContainer* excludeCC)
{
	for (auto& tc : controllables)
	{
		String tName = searchNiceName ? tc->niceName : tc->shortName;
		if (tc != excludeC && tName == name)
		{
			return true;
		}
	}
	for (auto& tcc : controllableContainers)
	{
		String tName = searchNiceName ? tcc->niceName : tcc->shortName;
		if (tcc != excludeCC && tName == name)
		{
			return true;
		}
	}

	return false;
}

String ControllableContainer::getUniqueNameInContainer(const String& sourceName, bool searchNiceName, int suffix)
{
	String separator = searchNiceName ? " " : "_";
	String resultName = sourceName;
	if (suffix > 0)
	{
		StringArray sa;
		sa.addTokens(resultName, separator,"");
		if (sa.size() > 1 && (sa[sa.size() - 1].getIntValue() != 0 || sa[sa.size() - 1].containsOnly("0")))
		{
			int num = sa[sa.size() - 1].getIntValue() + suffix;
			sa.remove(sa.size() - 1);
			sa.add(String(num));
			resultName = sa.joinIntoString(separator);
		}
		else
		{
			resultName += separator + String(suffix);
		}
	}

	if (getControllableByName(resultName, true) != nullptr)
	{
		return getUniqueNameInContainer(sourceName, searchNiceName, suffix + 1);
	}

	if (getControllableContainerByName(resultName, true) != nullptr)
	{
		return getUniqueNameInContainer(sourceName, searchNiceName, suffix + 1);
	}

	return resultName;
}

void ControllableContainer::updateLiveScriptObjectInternal(DynamicObject* parent)
{
	ScriptTarget::updateLiveScriptObjectInternal(parent);


	bool transferToParent = parent != nullptr;

	
	for (auto& cc : controllableContainers)
	{
		if (cc == nullptr || cc.wasObjectDeleted() || cc->shortName.isEmpty()) continue;
		if (!cc->includeInScriptObject) continue;

		if (transferToParent) parent->setProperty(cc->shortName, cc->getScriptObject());
		else liveScriptObject->setProperty(cc->shortName, cc->getScriptObject());

	}
	


	
	for (auto& c : controllables)
	{
		if (c == nullptr || c->shortName.isEmpty()) return;
		if (!c->includeInScriptObject) continue;
		if (transferToParent) parent->setProperty(c->shortName, c->getScriptObject());
		else liveScriptObject->setProperty(c->shortName, c->getScriptObject());
	}
	

	liveScriptObject->setProperty("name", shortName);
	liveScriptObject->setProperty("niceName", niceName);


}

var ControllableContainer::getChildFromScript(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return var();
	ControllableContainer* m = getObjectFromJS<ControllableContainer>(a);
	if (m == nullptr) return var();
	String nameToFind = a.arguments[0].toString();
	ControllableContainer* cc = m->getControllableContainerByName(nameToFind);
	if (cc != nullptr) return cc->getScriptObject();

	Controllable* c = m->getControllableByName(nameToFind);
	if (c != nullptr) return c->getScriptObject();

	LOG("Child not found from script " + a.arguments[0].toString());
	return var();
}

var ControllableContainer::getParentFromScript(const juce::var::NativeFunctionArgs& a)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(a);

	int level = a.numArguments > 0 ? (int)a.arguments[0] : 1;
	ControllableContainer* target = cc->parentContainer;
	if (target == nullptr) return var();
	for (int i = 1; i < level; ++i)
	{
		target = target->parentContainer;
		if (target == nullptr) return var();
	}

	return target->getScriptObject();
}

var ControllableContainer::setNameFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return var();
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(a);
	cc->setNiceName(a.arguments[0].toString());
	if (a.numArguments >= 2) cc->setCustomShortName(a.arguments[1].toString());
	else cc->setAutoShortName();

	return var();
}

var ControllableContainer::setCollapsedFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return var();
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(a);
	cc->editorIsCollapsed = (int)a.arguments[0] > 0;
	cc->queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerCollapsedChanged, cc));
	return var();
}

var ControllableContainer::addTriggerFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 2)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();
	
	return cc->addTrigger(args.arguments[0], args.arguments[1])->getScriptObject();
}

var ControllableContainer::addBoolParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 3)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	return cc->addBoolParameter(args.arguments[0], args.arguments[1], (bool)args.arguments[2])->getScriptObject();
}

var ControllableContainer::addIntParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 3)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	return cc->addIntParameter(args.arguments[0], args.arguments[1], (int)args.arguments[2], args.numArguments >= 4?(int)args.arguments[3]:INT32_MIN, args.numArguments >= 5?(int)args.arguments[4]:INT32_MAX)->getScriptObject();
}

var ControllableContainer::addFloatParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 3)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	return cc->addFloatParameter(args.arguments[0], args.arguments[1], (float)args.arguments[2], args.numArguments >= 4 ? (int)args.arguments[3] : INT32_MIN, args.numArguments >= 5 ? (int)args.arguments[4] : INT32_MAX)->getScriptObject();
}

var ControllableContainer::addStringParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 3)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	return cc->addStringParameter(args.arguments[0], args.arguments[1], args.arguments[2])->getScriptObject();
}

var ControllableContainer::addEnumParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 2)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	EnumParameter * p = cc->addEnumParameter(args.arguments[0], args.arguments[1]);
	int numOptions = (int)floor((args.numArguments - 2) / 2.0f);
	for (int i = 0; i < numOptions; ++i)
	{
		int optionIndex = 2 + i * 2;
		p->addOption(args.arguments[optionIndex].toString(), args.arguments[optionIndex + 1]);
	}

	return p->getScriptObject();
}

var ControllableContainer::addTargetParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 2)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	TargetParameter * tp = cc->addTargetParameter(args.arguments[0], args.arguments[1]);
	if (args.numArguments >= 3)
	{
		bool isContainer = (int)args.arguments[2] > 0;
		if (isContainer) tp->targetType = TargetParameter::CONTAINER;
	}

	return tp->getScriptObject();
}

var ControllableContainer::addColorParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 3)) return var();

	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	var color;
	if (args.arguments[2].isArray())
	{
		color = args.arguments[2];
		while (color.size() < 4) color.append(color.size() < 3 ? 0 : 1);
		for (int i = 0; i < color.size(); ++i) color[i] = (float)color[i] * 255;
	}
	else if (args.numArguments >= 5)
	{
		color.append((float)args.arguments[2] * 255);
		color.append((float)args.arguments[3] * 255);
		color.append((float)args.arguments[4] * 255);
		color.append((float)args.numArguments >= 6 ? (float)args.arguments[5] * 255 : 255);
	}
	else if (args.arguments[2].isInt() || args.arguments[2].isInt64())
	{
		color.append(((int)args.arguments[2] >> 24) & 0xFF);
		color.append(((int)args.arguments[2] >> 16) & 0xFF);
		color.append(((int)args.arguments[2] >> 8) & 0xFF);
		color.append(((int)args.arguments[2]) & 0xFF);
	}

	return cc->addColorParameter(args.arguments[0], args.arguments[1], Colour((uint8)(int)color[0], (uint8)(int)color[1], (uint8)(int)color[2], (uint8)(int)color[3]))->getScriptObject();
}

var ControllableContainer::addPoint2DParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 2)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	return cc->addPoint2DParameter(args.arguments[0], args.arguments[1])->getScriptObject();
}

var ControllableContainer::addPoint3DParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 2)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	return cc->addPoint3DParameter(args.arguments[0], args.arguments[1])->getScriptObject();
}

var ControllableContainer::addFileParameterFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 2)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c != nullptr) return c->getScriptObject();

	FileParameter * fp = cc->addFileParameter(args.arguments[0], args.arguments[1]);
	fp->directoryMode = args.numArguments > 2 ? ((int)args.arguments[2] > 0) : false;
	return fp->getScriptObject();
}

var ControllableContainer::addContainerFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 1)) return var();
	
	ControllableContainer* newCC = cc->getControllableContainerByName(args.arguments[0], true, false);
	if (newCC != nullptr) return newCC->getScriptObject();

	newCC = new ControllableContainer(args.arguments[0]);
	cc->addChildControllableContainer(newCC, true);
	return newCC->getScriptObject();
}

var ControllableContainer::removeContainerFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 1)) return var();
	
	ControllableContainer* removeCC = cc->getControllableContainerByName(args.arguments[0], true, false);
	if (removeCC == nullptr) return var();
	
	cc->removeChildControllableContainer(removeCC);
	return var();
}

var ControllableContainer::removeControllableFromScript(const var::NativeFunctionArgs & args)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(args);
	if (!checkNumArgs(cc->niceName, args, 1)) return var();
	
	Controllable* c = cc->getControllableByName(args.arguments[0], true, false);
	if (c == nullptr) return var();
	
	cc->removeControllable(c);
	return var();
}

var ControllableContainer::getControlAddressFromScript(const juce::var::NativeFunctionArgs& a)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(a);
	if (cc == nullptr) return var();
	ControllableContainer* ref = nullptr;
	if (a.numArguments > 0 && a.arguments[0].isObject())
	{
		if (DynamicObject* d = a.thisObject.getDynamicObject())
		{
			ref = dynamic_cast<ControllableContainer*>((ControllableContainer*)(int64)d->getProperty(scriptPtrIdentifier));
		}
	}

	return cc->getControlAddress(ref);
}

var ControllableContainer::getScriptControlAddressFromScript(const juce::var::NativeFunctionArgs& a)
{
	ControllableContainer* cc = getObjectFromJS<ControllableContainer>(a);
	return "root" + cc->getControlAddress().replaceCharacter('/', '.');
}


bool ControllableContainer::checkNumArgs(const String &logName, const var::NativeFunctionArgs & args, int expectedArgs)
{
	if (args.numArguments < expectedArgs)
	{
		NLOGERROR(logName, "Error, function takes at least" + String(expectedArgs) + " arguments, got " + String(args.numArguments));
		if (args.numArguments > 0) NLOG("", " > When trying to add : " + args.arguments[0].toString());
		return false;
	}

	return true;
}

String ControllableContainer::getScriptTargetString()
{
	return "[" + niceName + ": Container]";
}

InspectableEditor* ControllableContainer::getEditor(bool isRoot)
{
	if (customGetEditorFunc != nullptr) return customGetEditorFunc(this, isRoot);
	return new GenericControllableContainerEditor(this, isRoot);
}

DashboardItem* ControllableContainer::createDashboardItem()
{
	return new DashboardCCItem(this);
}

EnablingControllableContainer::EnablingControllableContainer(const String& n, bool _canBeDisabled) :
	ControllableContainer(n),
	enabled(nullptr),
	canBeDisabled(false)
{
	setCanBeDisabled(_canBeDisabled);

}

void EnablingControllableContainer::setCanBeDisabled(bool value)
{
	if (canBeDisabled == value) return;

	canBeDisabled = value;

	if (canBeDisabled)
	{
		enabled = addBoolParameter("Enabled", "Enable or disable this element.\nMost of the time it will mean stopping the process of what's going on inside it.", true);
		enabled->hideInEditor = true;
	}
	else
	{
		removeControllable(enabled);
		enabled = nullptr;
	}
}

InspectableEditor* EnablingControllableContainer::getEditor(bool isRoot)
{
	if (customGetEditorFunc != nullptr) return customGetEditorFunc(this, isRoot);
	return new EnablingControllableContainerEditor(this, isRoot);
}

ControllableContainer* ControllableContainer::ControllableContainerAction::getControllableContainer()
{
	if (containerRef != nullptr && !containerRef.wasObjectDeleted()) return containerRef.get();
	else if (Engine::mainEngine != nullptr)
	{
		ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(controlAddress, true);
		return cc;
	}

	return nullptr;
}


bool ControllableContainer::ControllableContainerChangeNameAction::perform()
{
	ControllableContainer* cc = getControllableContainer();
	if (cc != nullptr)
	{
		cc->setNiceName(newName);
		return true;
	}
	return false;
}

bool ControllableContainer::ControllableContainerChangeNameAction::undo()
{
	ControllableContainer* cc = getControllableContainer();
	if (cc != nullptr)
	{
		cc->setNiceName(oldName);
		return true;
	}
	return false;
}

ControllableContainer::ControllableContainerControllableAction::ControllableContainerControllableAction(ControllableContainer* cc, Controllable* c) :
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

Controllable* ControllableContainer::ControllableContainerControllableAction::getItem()
{
	if (cRef != nullptr && !cRef.wasObjectDeleted()) return dynamic_cast<Controllable*>(cRef.get());
	else
	{
		ControllableContainer* cc = this->getControllableContainer();
		if (cc != nullptr) return cc->getControllableByName(cShortName);
	}

	return nullptr;
}

bool ControllableContainer::AddControllableAction::perform()
{
	ControllableContainer* cc = this->getControllableContainer();
	if (cc == nullptr)
	{
		return false;
	}

	Controllable* c = this->getItem();
	if (c != nullptr)
	{
		cc->addControllable(c);
	}
	else
	{
		c = ControllableFactory::createControllable(cType);
	}

	if (c == nullptr) return false;

	this->cShortName = c->shortName;
	return true;
}

bool ControllableContainer::AddControllableAction::undo()
{
	Controllable* c = this->getItem();
	if (c == nullptr) return false;
	data = c->getJSONData();
	ControllableContainer* cc = getControllableContainer();
	if (cc != nullptr)
	{
		cc->removeControllable(c);
		cRef = nullptr;
	}
	return true;
}

ControllableContainer::RemoveControllableAction::RemoveControllableAction(ControllableContainer* cc, Controllable* c) :
	ControllableContainerControllableAction(cc, c)
{
}

bool ControllableContainer::RemoveControllableAction::perform()
{
	Controllable* c = this->getItem();

	if (c == nullptr) return false;
	getControllableContainer()->removeControllable(c);
	cRef = nullptr;
	return true;
}

bool ControllableContainer::RemoveControllableAction::undo()
{
	ControllableContainer* cc = getControllableContainer();
	if (cc == nullptr) return false;
	Controllable* c = ControllableFactory::createControllable(cType);
	if (c != nullptr)
	{
		c->loadJSONData(data);
		cc->addControllable(c);
		cRef = c;
	}
	return true;
}
