#include "Controllable.h"
/*
  ==============================================================================

	Controllable.cpp
	Created: 8 Mar 2016 1:08:56pm
	Author:  bkupe

  ==============================================================================
*/

const Array<String> Controllable::typeNames = { 
	"Custom", 
	Trigger::getTypeStringStatic(),
	FloatParameter::getTypeStringStatic(),
	IntParameter::getTypeStringStatic(),
	BoolParameter::getTypeStringStatic(),
	StringParameter::getTypeStringStatic(),
	EnumParameter::getTypeStringStatic(),
	Point2DParameter::getTypeStringStatic(),
	Point3DParameter::getTypeStringStatic(),
	TargetParameter::getTypeStringStatic(),
	ColorParameter::getTypeStringStatic()
};

Controllable::Controllable(const Type &type, const String & niceName, const String &description, bool enabled) :
	ScriptTarget("", this, "Controllable"),
	type(type),
	niceName(""),
	shortName(""),
	description(description),
	enabled(true),
	canBeDisabledByUser(false),
	descriptionIsEditable(false),
	hasCustomShortName(false),
	isControllableFeedbackOnly(false),
	includeInScriptObject(true),
	isSavable(true),
	saveValueOnly(true),
	isLoadingData(false),
	isControlledByParrot(false),
	isCustomizableByUser(false),
	isRemovableByUser(false),
	userCanSetReadOnly(false),
	replaceSlashesInShortName(true),
	dashboardDefaultLabelParentLevel(0), 
	dashboardDefaultAppendLabel(false),
	parentContainer(nullptr),
	queuedNotifier(10)
{
	scriptObject.setMethod("isParameter", Controllable::checkIsParameterFromScript);
	scriptObject.setMethod("getParent", Controllable::getParentFromScript);
	scriptObject.setMethod("setName", Controllable::setNameFromScript);
	scriptObject.setMethod("setAttribute", Controllable::setAttributeFromScript);
	scriptObject.setMethod("getControlAddress", Controllable::getControlAddressFromScript);
	scriptObject.setMethod("getScriptControlAdress", Controllable::getScriptControlAddressFromScript);


	setEnabled(enabled);
	setNiceName(niceName);
}

Controllable::~Controllable() {
	queuedNotifier.cancelPendingUpdate();
	Controllable::masterReference.clear();
}

UndoableAction * Controllable::setUndoableNiceName(const String & newNiceName, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		setNiceName(newNiceName);
		return nullptr;
	}

	UndoableAction * a = new ControllableChangeNameAction(this, niceName, newNiceName);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Rename " + niceName, a);
	return a;

	//if Main Engine loading, just set the value without undo history
	
}

void Controllable::setNiceName(const String & _niceName) {
	if (niceName == _niceName) return;

	this->niceName = _niceName;
	if (!hasCustomShortName) setAutoShortName();
	else
	{
		listeners.call(&Listener::controllableNameChanged, this);
		queuedNotifier.addMessage(new ControllableEvent(ControllableEvent::NAME_CHANGED, this));
	}
}

void Controllable::setCustomShortName(const String & _shortName)
{
	this->shortName = _shortName;
	hasCustomShortName = true;
	scriptTargetName = shortName;
	updateControlAddress();
	listeners.call(&Listener::controllableNameChanged, this);
	queuedNotifier.addMessage(new ControllableEvent(ControllableEvent::NAME_CHANGED, this));
	
}

void Controllable::setAutoShortName() {
	hasCustomShortName = false;
	shortName = StringUtil::toShortName(niceName, replaceSlashesInShortName);
	if (shortName.isEmpty()) shortName = "***";
	scriptTargetName = shortName;
	updateControlAddress();
	listeners.call(&Listener::controllableNameChanged, this);
	queuedNotifier.addMessage(new ControllableEvent(ControllableEvent::NAME_CHANGED, this));
}


void Controllable::setEnabled(bool value, bool silentSet, bool force)
{
	if (!force && value == enabled) return;
	enabled = value;
	if (!silentSet) notifyStateChanged();
}

void Controllable::setControllableFeedbackOnly(bool value)
{
	if (isControllableFeedbackOnly == value) return;
	isControllableFeedbackOnly = value;
	listeners.call(&Listener::controllableFeedbackStateChanged, this);
	queuedNotifier.addMessage(new ControllableEvent(ControllableEvent::FEEDBACK_STATE_CHANGED, this));
}

void Controllable::notifyStateChanged()
{
	listeners.call(&Listener::controllableStateChanged, this);
	queuedNotifier.addMessage(new ControllableEvent(ControllableEvent::STATE_CHANGED, this));
}

void Controllable::setParentContainer(ControllableContainer * container)
{
	this->parentContainer = container;
	updateControlAddress();
}

void Controllable::updateControlAddress()
{
	this->controlAddress = getControlAddress();
	this->liveScriptObjectIsDirty = true;
	listeners.call(&Listener::controllableControlAddressChanged, this);
	queuedNotifier.addMessage(new ControllableEvent(ControllableEvent::CONTROLADDRESS_CHANGED, this));
}

void Controllable::remove(bool addToUndo)
{
	listeners.call(&Controllable::Listener::askForRemoveControllable, this, addToUndo);
}

void Controllable::updateLiveScriptObjectInternal(DynamicObject * parent)
{
	liveScriptObject->setProperty("name", shortName);
	liveScriptObject->setProperty("niceName", niceName);
}

bool Controllable::shouldBeSaved()
{
	if (hideInRemoteControl != defaultHideInRemoteControl) return true;
	if (userCanSetReadOnly && isControllableFeedbackOnly) return true;
	if (!isSavable) return false;
	if (!saveValueOnly) return true;
	if (saveCustomData && !customData.isVoid()) return true;
	return false;
}


var Controllable::getJSONData(ControllableContainer * relativeTo)
{
	var data = getJSONDataInternal();
	data.getDynamicObject()->setProperty("controlAddress", getControlAddress(relativeTo));
	
	if (canBeDisabledByUser) data.getDynamicObject()->setProperty("enabled", enabled);
	if (hideInRemoteControl != defaultHideInRemoteControl) data.getDynamicObject()->setProperty("hideInRemoteControl", hideInRemoteControl);
	if(userCanSetReadOnly) data.getDynamicObject()->setProperty("feedbackOnly", isControllableFeedbackOnly);
	if (saveCustomData && !customData.isVoid()) data.getDynamicObject()->setProperty("customData", customData);

	if (saveValueOnly) return data;
	
	data.getDynamicObject()->setProperty("type", getTypeString());
	data.getDynamicObject()->setProperty("niceName", niceName);
	if (canBeDisabledByUser) data.getDynamicObject()->setProperty("enabled", enabled);
	data.getDynamicObject()->setProperty("customizable", isCustomizableByUser);
	data.getDynamicObject()->setProperty("removable", isRemovableByUser);
	data.getDynamicObject()->setProperty("description", description);
	data.getDynamicObject()->setProperty("hideInEditor", hideInEditor);
	data.getDynamicObject()->setProperty("feedbackOnly", isControllableFeedbackOnly);

	if (hasCustomShortName) data.getDynamicObject()->setProperty("shortName", shortName);

	return data;
}

void Controllable::loadJSONData(var data)
{
	if (data.isVoid() || !data.isObject()) return;
	
	isLoadingData = true;
	
	if (data.getDynamicObject()->hasProperty("type")) saveValueOnly = false;
	if (data.getDynamicObject()->hasProperty("niceName")) setNiceName(data.getProperty("niceName", ""));
	if (data.getDynamicObject()->hasProperty("shortName")) setCustomShortName(data.getProperty("shortName", ""));
	if (data.getDynamicObject()->hasProperty("enabled")) setEnabled(data.getProperty("enabled", enabled));
	isCustomizableByUser = data.getProperty("customizable", isCustomizableByUser);
	isRemovableByUser = data.getProperty("removable", isRemovableByUser);
	description = data.getProperty("description", description);
	hideInEditor = data.getProperty("hideInEditor", hideInEditor);
	hideInRemoteControl = data.getProperty("hideInRemoteControl", defaultHideInRemoteControl);
	if (data.getDynamicObject()->hasProperty("feedbackOnly")) setControllableFeedbackOnly(data.getProperty("feedbackOnly",false));
	customData = data.getProperty("customData", customData);

	loadJSONDataInternal(data);

	isLoadingData = false;
}

void Controllable::setupFromJSONData(var data)
{
	if (data.hasProperty("shortName")) setCustomShortName(data.getProperty("shortName", "[error]").toString());
	if (data.hasProperty("description")) description = data.getProperty("description", "[no description]").toString();

	StringArray attributes = getValidAttributes();
	for (auto& a : attributes)
	{
		if (data.hasProperty(a)) setAttribute(a, data.getDynamicObject()->getProperty(a));
	}
}

String Controllable::getControlAddress(ControllableContainer * relativeTo)
{
	return parentContainer->getControlAddress(relativeTo) + "/" + shortName;
}

InspectableEditor * Controllable::getEditor(bool isRootEditor) {

	return new ControllableEditor(this, isRootEditor);
}

ControllableDetectiveWatcher* Controllable::getDetectiveWatcher()
{
	return new ControllableDetectiveWatcher();
}

DashboardItem * Controllable::createDashboardItem()
{
	return new DashboardControllableItem(this);
}

String Controllable::getDefaultDashboardLabel() const
{
	if (dashboardDefaultLabelParentLevel == 0) return niceName;
	String s = "";
	ControllableContainer* cParent = parentContainer;
	for (int i = 0; i < dashboardDefaultLabelParentLevel-1; i++)
	{
		if (cParent != nullptr) cParent = cParent->parentContainer;
	}
	if (cParent != nullptr) s = cParent->niceName;
	if (dashboardDefaultAppendLabel) s += " : " + niceName;
	return s.isNotEmpty() ? s : niceName;
}

void Controllable::setAttribute(String param, var value)
{
	if (param == "description") description = value;
	else if (param == "readonly" || param == "readOnly") setControllableFeedbackOnly(value);
	else if (param == "enabled") setEnabled(value);
}

StringArray Controllable::getValidAttributes() const
{
	return { "targetType", "searchLevel", "allowedTypes", "excludedTypes","root", "labelLevel" };
}


//SCRIPT
var Controllable::setValueFromScript(const juce::var::NativeFunctionArgs& a) {

	Controllable * c = getObjectFromJS<Controllable>(a);
	bool success = false;

	if (c != nullptr)
	{

		success = true;

		if (a.numArguments == 0) {
			if (c->type == Controllable::Type::TRIGGER) ((Trigger*)c)->trigger();
			else
			{
				LOGWARNING("setValue needs 1 argument for parameters");
			}
		}
		else
		{
			var value = a.arguments[0];
			bool valueIsABool = value.isBool();
			bool valueIsANumber = value.isDouble() || value.isInt() || value.isInt64();

			switch (c->type)
			{
			case Controllable::Type::TRIGGER:
				if (valueIsABool)
				{
					if ((bool)value) ((Trigger *)c)->trigger();
				} else if (valueIsANumber)
				{
					if ((float)value >= 1) ((Trigger *)c)->trigger();
				}

				break;

			case Controllable::Type::BOOL:

				if (valueIsABool) ((BoolParameter *)c)->setValue((bool)value);
				else if (valueIsANumber) ((BoolParameter *)c)->setValue((float)value > .5);
				break;

			case Controllable::Type::FLOAT:
				if (valueIsABool || valueIsANumber) ((FloatParameter *)c)->setValue(value);
				break;
			case Controllable::Type::INT:
				if (valueIsABool || valueIsANumber) ((IntParameter *)c)->setValue(value);
				break;

			case Controllable::Type::STRING:
				if (value.isString()) ((StringParameter *)c)->setValue(value);
				break;

			case Controllable::Type::COLOR:
				if (value.isArray() && value.size() >= 3)
				{
					((ColorParameter *)c)->setColor(Colour::fromFloatRGBA((float)(value[0]), 
						(float)(value[1]),
						(float)(value[2]),
						value.size() > 3 ? (float)(value[3]) : 1));
				}
				else if (a.numArguments >= 3)
				{
					((ColorParameter*)c)->setColor(Colour::fromFloatRGBA((float)(a.arguments[0]),
						(float)(a.arguments[1]),
						(float)(a.arguments[2]),
						a.numArguments > 3 ? (float)(a.arguments[3]) : 1));
				}
				else if (a.numArguments == 1 && valueIsANumber)
				{
					((ColorParameter*)c)->setColor(Colour((int)a.arguments[0]));
				}
				break;

			case Controllable::Type::POINT2D:
				if (value.isArray() && value.size() >= 2) ((Point2DParameter *)c)->setPoint((float)value[0], (float)value[1]);
				else if (a.numArguments >= 2) ((Point2DParameter*)c)->setPoint((float)a.arguments[0], (float)a.arguments[1]);
				break;

			case Controllable::Type::POINT3D:
				if (value.isArray() && value.size() >= 3) ((Point3DParameter *)c)->setVector((float)value[0], (float)value[1], (float)value[2]);
				else if (a.numArguments >= 3) ((Point3DParameter*)c)->setVector((float)a.arguments[0], (float)a.arguments[1], (float)a.arguments[2]);
				break;

			case Controllable::Type::ENUM:
				((EnumParameter*)c)->setValueWithKey(value.toString());
				break;

			case Controllable::Type::TARGET:
				if (value.isObject())
				{
					TargetParameter* tp = (TargetParameter*)c;
					if (tp->targetType == TargetParameter::CONTROLLABLE)
					{
						Controllable * target = static_cast<Controllable *>((Controllable*)(int64)value.getDynamicObject()->getProperty(scriptPtrIdentifier));
						if (target != nullptr) tp->setValueFromTarget((Controllable *)target);
					}else
					{
						ControllableContainer * target = static_cast<ControllableContainer*>((ControllableContainer*)(int64)value.getDynamicObject()->getProperty(scriptPtrIdentifier));
						if (target != nullptr) tp->setValueFromTarget(target);
					}
				}
				else
				{
					LOGWARNING("Set target from script, value is not an object");
				}
				break;

			default:
				success = false;
				break;

			}
		}
	}

	return var();
}


var Controllable::checkIsParameterFromScript(const juce::var::NativeFunctionArgs& a) {

	Controllable * c = getObjectFromJS<Controllable>(a);
	return c->type != TRIGGER;
}

var Controllable::getParentFromScript(const juce::var::NativeFunctionArgs & a)
{
	Controllable * c = getObjectFromJS<Controllable>(a);
	int level = a.numArguments > 0 ? (int)a.arguments[0] : 1;
	ControllableContainer* target = c->parentContainer;
	if (target == nullptr) return var();
	for (int i = 1; i < level; ++i)
	{
		target = target->parentContainer;
		if (target == nullptr) return var();
	}
	return target->getScriptObject();

}

var Controllable::setNameFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return var();
	Controllable * c = getObjectFromJS<Controllable>(a);
	c->setNiceName(a.arguments[0].toString());
	if (a.numArguments >= 2) c->setCustomShortName(a.arguments[1].toString());
	else c->setAutoShortName();
	
	return var(); 
}

var Controllable::setAttributeFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (a.numArguments < 2) return var();
	Controllable* c = getObjectFromJS<Controllable>(a);
	if (c == nullptr) return var();
	c->setAttribute(a.arguments[0].toString(), a.arguments[1]);
	return var();
}

var Controllable::getControlAddressFromScript(const juce::var::NativeFunctionArgs& a)
{
	Controllable* c = getObjectFromJS<Controllable>(a);
	if (c == nullptr) return var();
	ControllableContainer* ref = nullptr;
	if (a.numArguments > 0 && a.arguments[0].isObject())
	{
		if (DynamicObject* d = a.thisObject.getDynamicObject())
		{
			ref = dynamic_cast<ControllableContainer*>((ControllableContainer*)(int64)d->getProperty(scriptPtrIdentifier));
		}
	}

	return c->getControlAddress(ref);
}

var Controllable::getScriptControlAddressFromScript(const juce::var::NativeFunctionArgs& a)
{
	Controllable* c = getObjectFromJS<Controllable>(a);
	if (c == nullptr) return var();
	return "root" + c->controlAddress.replaceCharacter('/', '.');
}

String Controllable::getScriptTargetString()
{
	return "[" + niceName + " : " + getTypeString() + "]";
}

String Controllable::getWarningTargetName() const 
{ 
	if (warningResolveInspectable != nullptr)
	{
		ControllableContainer* cc = dynamic_cast<ControllableContainer*>(warningResolveInspectable);
		if (cc != nullptr) return cc->niceName + " > " + niceName;
		else return niceName;
	}

	return niceName;
}

Controllable * Controllable::ControllableAction::getControllable()
{
	if (controllableRef != nullptr && !controllableRef.wasObjectDeleted()) return controllableRef.get();
	else if(Engine::mainEngine != nullptr)
	{
		Controllable * c = Engine::mainEngine->getControllableForAddress(controlAddress, true);
		return c;
	}

	return nullptr;
}

bool Controllable::ControllableChangeNameAction::perform()
{
	Controllable * c = getControllable();
	if (c != nullptr)
	{
		c->setNiceName(newName);
		return true;
	}
	return false;
}

bool Controllable::ControllableChangeNameAction::undo()
{
	Controllable * c = getControllable();
	if (c != nullptr)
	{
		c->setNiceName(oldName); 
		return true;
	}
	return false;
}
