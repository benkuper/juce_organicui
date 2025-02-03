/*
  ==============================================================================

	EnumParameter.cpp
	Created: 29 Sep 2016 5:34:59pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "EnumParameter.h"

EnumParameter::EnumParameter(const String& niceName, const String& description, bool enabled) :
	Parameter(Type::ENUM, niceName, description, "", var(), var(), enabled),
	enumParameterNotifier(5)
{
	//lockManualControlMode = true;

	scriptObject.getDynamicObject()->setMethod("getKey", EnumParameter::getValueKeyFromScript);
	scriptObject.getDynamicObject()->setMethod("setData", EnumParameter::setValueWithDataFromScript);
	scriptObject.getDynamicObject()->setMethod("setNext", EnumParameter::setNextFromScript);
	scriptObject.getDynamicObject()->setMethod("setPrevious", EnumParameter::setPreviousFromScript);
	scriptObject.getDynamicObject()->setMethod("addOption", EnumParameter::addOptionFromScript);
	scriptObject.getDynamicObject()->setMethod("setOptions", EnumParameter::setOptionsFromScript);
	scriptObject.getDynamicObject()->setMethod("removeOptions", EnumParameter::removeOptionsFromScript);
	scriptObject.getDynamicObject()->setMethod("getAllOptions", EnumParameter::getAllOptionsFromScript);
	scriptObject.getDynamicObject()->setMethod("getOptionAt", EnumParameter::getOptionAtFromScript);
	scriptObject.getDynamicObject()->setMethod("getIndex", EnumParameter::getIndexFromScript);

	value = "";
}

EnumParameter::~EnumParameter()
{
	if (enumParameterNotifier.isUpdatePending())
	{
		MessageManagerLock mmLock(Thread::getCurrentThread());
		if (mmLock.lockWasGained()) enumParameterNotifier.handleUpdateNowIfNeeded();
		enumParameterNotifier.cancelPendingUpdate();
	}
}

EnumParameter* EnumParameter::addOption(String key, var data, bool selectIfFirstOption)
{
	GenericScopedLock lock(enumValues.getLock());
	enumValues.add(new EnumValue(key, data));
	if (enumValues.size() == 1 && selectIfFirstOption)
	{
		defaultValue = key;
		setValue(key, true, false, false);
	}

	enumListeners.call(&EnumParameterListener::enumOptionAdded, this, key);
	enumParameterNotifier.addMessage(new EnumParameterEvent(EnumParameterEvent::ENUM_OPTION_ADDED, this));
	updateArgDescription();
	return this;
}

void EnumParameter::updateOption(int index, String key, var data, bool addIfNotThere)
{
	GenericScopedLock lock(enumValues.getLock());
	if (index >= enumValues.size())
	{
		if (!addIfNotThere) return;
		while (index > enumValues.size()) addOption("#" + String(enumValues.size()), var());
		addOption(key, data);
		return;
	}

	String k = enumValues[index]->key;
	if (k == key && enumValues[index]->value == data) return;

	enumValues.set(index, new EnumValue(key, data));
	enumListeners.call(&EnumParameterListener::enumOptionUpdated, this, index, k, key);
	enumParameterNotifier.addMessage(new EnumParameterEvent(EnumParameterEvent::ENUM_OPTION_UPDATED, this));
	updateArgDescription();
}

void EnumParameter::removeOption(String key)
{
	GenericScopedLock lock(enumValues.getLock());
	enumValues.remove(getIndexForKey(key));
	enumListeners.call(&EnumParameterListener::enumOptionRemoved, this, key);
	enumParameterNotifier.addMessage(new EnumParameterEvent(EnumParameterEvent::ENUM_OPTION_REMOVED, this));
	updateArgDescription();

	if (getValueKey() == key) setValue("");
}

void EnumParameter::setOptions(Array<EnumValue> options)
{
	GenericScopedLock lock(enumValues.getLock());
	for (int i = 0; i < options.size(); i++)
	{
		updateOption(i, options[i].key, options[i].value, true);
	}

	while (enumValues.size() > options.size()) removeOption(enumValues[enumValues.size() - 1]->key);
}

void EnumParameter::clearOptions()
{
	GenericScopedLock lock(enumValues.getLock());
	StringArray keysToRemove;
	for (auto& ev : enumValues) keysToRemove.add(ev->key);
	for (auto& k : keysToRemove) removeOption(k);
}

void EnumParameter::updateArgDescription()
{
	argumentsDescription = "";
	for (int i = 0; i < enumValues.size(); ++i)
	{
		argumentsDescription += enumValues[i]->key;
		if (i < enumValues.size() - 1) argumentsDescription += " | ";
	}
}

var EnumParameter::getValue()
{
	return getValueData();
}

var EnumParameter::getValueData() {
	GenericScopedLock lock(valueSetLock);
	return curEnumValue.value;
}

String EnumParameter::getValueKey() {
	GenericScopedLock lock(valueSetLock);
	return value.toString();
}

int EnumParameter::getIndexForKey(StringRef key)
{
	GenericScopedLock lock(enumValues.getLock());
	int numValues = enumValues.size();
	for (int i = 0; i < numValues; ++i) if (enumValues[i]->key == key) return i;
	return -1;
}

EnumParameter::EnumValue* EnumParameter::getEntryForKey(StringRef key)
{
	int index = getIndexForKey(key);
	if (index == -1) return nullptr;
	return enumValues[index];
}

StringArray EnumParameter::getAllKeys()
{
	StringArray result;
	for (auto& ev : enumValues) result.add(ev->key);
	return result;
}

void EnumParameter::setValueInternal(juce::var& newValue)
{
	Parameter::setValueInternal(newValue);

	EnumValue* ev = getEntryForKey(value.toString());

	{
		GenericScopedLock lock(enumValues.getLock());
		if (ev == nullptr)
		{
			curEnumValue.key = "";
			curEnumValue.value = var();
		}
		else
		{
			curEnumValue.key = ev->key;
			curEnumValue.value = ev->value;
		}
	}
}

bool EnumParameter::setValueWithData(var data)
{
	String key = "";

	{
		GenericScopedLock lock(enumValues.getLock());
		for (auto& ev : enumValues)
		{
			setValueWithKey(ev->key);
			return true;
		}
	}

	return false;
}

bool EnumParameter::setValueWithKey(String key)
{
	if (getEntryForKey(key) == nullptr) return false;
	setValue(key);
	return true;
}

bool EnumParameter::setValueAtIndex(int index)
{
	if (index >= enumValues.size()) return false;
	setValueWithKey(enumValues[index]->key);
	return true;
}

void EnumParameter::setPrev(bool loop, bool addToUndo)
{
	GenericScopedLock lock(enumValues.getLock());
	int targetIndex = getIndexForKey(value.toString()) - 1;
	if (targetIndex < 0)
	{
		if (loop) targetIndex = enumValues.size() - 1;
		else return;
	}

	String newValue = enumValues[targetIndex]->key;

	if (addToUndo) setUndoableValue(newValue, false, true);
	else setValueWithKey(newValue);
}

void EnumParameter::setNext(bool loop, bool addToUndo)
{

	GenericScopedLock lock(enumValues.getLock());
	int targetIndex = getIndexForKey(value.toString()) + 1;
	if (targetIndex >= enumValues.size())
	{
		if (loop) targetIndex = 0;
		else return;
	}

	String newValue = enumValues[targetIndex]->key;

	if (addToUndo) setUndoableValue(newValue, false, true);
	else setValueWithKey(newValue);
}

bool EnumParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	return oldValue.toString() == newValue.toString();
}

var EnumParameter::getJSONDataInternal()
{
	var data = Parameter::getJSONDataInternal();
	if (!saveValueOnly)
	{
		var optData(new DynamicObject());
		for (auto& e : enumValues) optData.getDynamicObject()->setProperty(e->key, e->value);
		data.getDynamicObject()->setProperty("enumOptions", optData);
	}
	return data;
}

void EnumParameter::loadJSONDataInternal(var data)
{
	if (!saveValueOnly)
	{
		var optData = data.getProperty("enumOptions", var());
		if (optData.isObject())
		{
			NamedValueSet props = optData.getDynamicObject()->getProperties();
			for (auto& p : props)
			{
				addOption(p.name.toString(), p.value, false);
			}
		}
	}
	Parameter::loadJSONDataInternal(data);
}

void EnumParameter::setupFromJSONData(var data)
{
	Parameter::setupFromJSONData(data);
	var optionsData = data.getProperty("options", var());
	if (optionsData.isObject())
	{
		NamedValueSet options = optionsData.getDynamicObject()->getProperties();
		for (auto& o : options)
		{
			addOption(o.name.toString(), o.value);
		}
	}
	else
	{
		LOGWARNING("Options property is not valid : " << optionsData.toString());
	}
}

var EnumParameter::getRemoteControlValue()
{
	return getValueKey();
}

var EnumParameter::getRemoteControlRange()
{
	var range = var();
	StringArray keys = getAllKeys();
	var enumRange;
	for (auto& k : keys) enumRange.append(k);
	var rData(new DynamicObject());
	rData.getDynamicObject()->setProperty("VALS", enumRange);
	range.append(rData);
	return range;
}

var EnumParameter::getValueKeyFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());
	return ep->getValueKey();
}

var EnumParameter::addOptionFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());

	if (a.numArguments < 2)
	{
		NLOGWARNING("Script", "EnumParameter addOption should at least have 2 arguments");
		return var();
	}

	ep->addOption(a.arguments[0].toString(), a.arguments[1]);

	return var();
}

juce::var EnumParameter::setOptionsFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());

	if (a.numArguments == 0)
	{
		NLOGWARNING("Script", "EnumParameter setOption should have at least 1 argument");
		return var();
	}

	if (a.arguments[0].isObject())
	{
		NamedValueSet nvSet = a.arguments[0].getDynamicObject()->getProperties();
		Array<EnumValue> newOptions;

		for (auto& nv : nvSet)
		{
			newOptions.add(EnumValue(nv.name.toString(), nv.value));
		}

		ep->setOptions(newOptions);
	}
	else
	{
		NLOGWARNING("Script", "EnumParameter setOption should have an object as first argument");
	}

	return var();
}

var EnumParameter::removeOptionsFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());
	ep->clearOptions();
	return var();
}

var EnumParameter::setValueWithDataFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());

	if (a.numArguments < 1)
	{
		NLOGWARNING("Script", "EnumParameter setData requires 1 argument !");
		return var();
	}

	ep->setValueWithData(a.arguments[0]);

	return var();
}

var EnumParameter::getAllOptionsFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());

	var result;

	for (auto& ev : ep->enumValues)
	{
		var eData(new DynamicObject());
		eData.getDynamicObject()->setProperty("key", ev->key);
		eData.getDynamicObject()->setProperty("value", ev->value);
		result.append(eData);
	}

	return result;
}

var EnumParameter::getOptionAtFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());

	if (a.numArguments < 1)
	{
		LOGWARNING("getOptionAt needs one argument");
		return var();
	}

	int index = a.arguments[0];

	if (index < 0 || index >= ep->enumValues.size())
	{
		LOGWARNING("getOptionAt, index  " << index << " out of range. (" << ep->enumValues.size() << " values)");
		return var();
	}

	var eData(new DynamicObject());
	eData.getDynamicObject()->setProperty("key", ep->enumValues[index]->key);
	eData.getDynamicObject()->setProperty("value", ep->enumValues[index]->value);

	return eData;
}

var EnumParameter::getIndexFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());
	return ep->getIndexForKey(ep->getValueKey());
}

var EnumParameter::setNextFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());
	ep->setNext(a.numArguments >= 1 ? (bool)(int)a.arguments[0] : false);
	return var();
}

var EnumParameter::setPreviousFromScript(const juce::var::NativeFunctionArgs& a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	EnumParameter* ep = dynamic_cast<EnumParameter*>(c.get());
	ep->setPrev(a.numArguments >= 1 ? (bool)(int)a.arguments[0] : false);
	return var();
}

EnumParameterUI* EnumParameter::createUI(Array<EnumParameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new EnumParameterUI(parameters);
}

EnumParameterButtonBarUI* EnumParameter::createButtonBarUI(Array<EnumParameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new EnumParameterButtonBarUI(parameters);
}

ControllableUI* EnumParameter::createDefaultUI(Array<Controllable*> controllables) {
	return createUI(getArrayAs<Controllable, EnumParameter>(controllables));
}

DashboardItem* EnumParameter::createDashboardItem()
{
	return new DashboardEnumParameterItem(this);
}
