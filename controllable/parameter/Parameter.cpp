/*
  ==============================================================================

	Parameter.cpp
	Created: 8 Mar 2016 1:08:19pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include <deque>
#include "Parameter.h"

using namespace juce;

namespace
{
struct ParameterBailOutChecker
{
	WeakReference<Parameter> parameter;
	bool shouldBailOut() const { return parameter == nullptr || parameter->isBeingDestroyed; }
};

// Preserve every captured value and its ordering, but do not post one OS
// message per animated parameter/tick. Short batches leave time for input,
// paint and timer messages even while interpolation is producing more values.
class ParameterValueDispatcher : public std::enable_shared_from_this<ParameterValueDispatcher>
{
public:
	void post(std::function<void()> callback)
	{
		{
			const ScopedLock guard(lock);
			pending.push_back(std::move(callback));
			if (deliveryPending) return;
			deliveryPending = true;
		}
		MessageManager::callAsync([self = shared_from_this()] { self->deliver(); });
	}

private:
	void deliver()
	{
		const auto deadline = Time::getMillisecondCounterHiRes() + 2.0;
		for (int count = 0; count < 128; ++count)
		{
			std::function<void()> callback;
			{
				const ScopedLock guard(lock);
				if (pending.empty()) { deliveryPending = false; return; }
				callback = std::move(pending.front());
				pending.pop_front();
			}
			callback();
			if (Time::getMillisecondCounterHiRes() >= deadline) break;
		}
		{
			const ScopedLock guard(lock);
			if (pending.empty()) { deliveryPending = false; return; }
		}
		// Reposting immediately can keep low-priority WM_PAINT/WM_TIMER messages
		// starved on Windows. A timer allows the message queue to become idle.
		Timer::callAfterDelay(1, [self = shared_from_this()] { self->deliver(); });
	}

	CriticalSection lock;
	std::deque<std::function<void()>> pending;
	bool deliveryPending = false;
};

std::shared_ptr<ParameterValueDispatcher> getParameterValueDispatcher()
{
	static const auto dispatcher = std::make_shared<ParameterValueDispatcher>();
	return dispatcher;
}
}

juce_ImplementSingleton(Parameter::ValueInterpolator::Manager);


Parameter::Parameter(const Type& type, const String& niceName, const String& description, var initialValue, var minValue, var maxValue, bool enabled) :
	Controllable(type, niceName, description, enabled),
	defaultValue(initialValue),
	value(initialValue),
	lastUndoValue(initialValue),
	canHaveRange(false),
	rebuildUIOnRangeChange(true),
	minimumValue(minValue),
	maximumValue(maxValue),
	lockManualControlMode(false),
	controlMode(MANUAL),
	alwaysNotify(false),
	referenceTarget(nullptr),
	referenceParameter(nullptr),
	canBeAutomated(false),
	isPresettable(true),
	isOverriden(false),
	forceSaveValue(false),
	forceSaveRange(false),
	isNotifyingChange(false),
	queuedNotifier(100),
	colorStatusMap(1)
{

	scriptObject.getDynamicObject()->setMethod("get", Parameter::getValueFromScript);
	scriptObject.getDynamicObject()->setMethod("set", Controllable::setValueFromScript);
	scriptObject.getDynamicObject()->setMethod("resetValue", Parameter::resetValueFromScript);
	scriptObject.getDynamicObject()->setMethod("getRange", Parameter::getRangeFromScript);
	scriptObject.getDynamicObject()->setMethod("setRange", Parameter::setRangeFromScript);
	scriptObject.getDynamicObject()->setMethod("hasRange", Parameter::hasRangeFromScript);
}

Parameter::~Parameter()
{
	if (referenceTarget != nullptr) referenceTarget->removeParameterListener(this); //avoid reassigning on deletion
	setReferenceParameter(nullptr);

	Parameter::masterReference.clear();
	queuedNotifier.cancelPendingUpdate();
	queuedNotifier.clearQueue();
}

void Parameter::setEnabled(bool _value, bool silentSet, bool force)
{
	Controllable::setEnabled(_value, silentSet, force);
	isOverriden = true;
}

void Parameter::setControlMode(ControlMode _mode)
{
	if (_mode == controlMode) return;

	controlMode = _mode;

	expression = nullptr;
	if (referenceTarget != nullptr)
	{
		referenceTarget->removeParameterListener(this);
		setReferenceParameter(nullptr);
	}
	referenceTarget = nullptr;
	automation = nullptr;

	switch (controlMode)
	{
	case MANUAL:
		break;

	case REFERENCE:
		referenceTarget.reset(new TargetParameter("Reference", "Reference for parameter " + niceName, ""));
		referenceTarget->lockManualControlMode = true;
		referenceTarget->addParameterListener(this);
		break;

	case EXPRESSION:
		expression.reset(new ScriptExpression(this));
		expression->setExpression(controlExpression);
		expression->addExpressionListener(this);
		break;

	case AUTOMATION:
		setControlAutomation();
		break;

	}

	WeakReference<Parameter> safeThis(this);
	parameterListeners.callChecked(ParameterBailOutChecker{ safeThis }, &ParameterListener::parameterControlModeChanged, this);
	if (auto* p = safeThis.get(); p != nullptr && !p->isBeingDestroyed)
		p->queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::CONTROLMODE_CHANGED, p));
}

void Parameter::setControlExpression(const String& e)
{
	controlExpression = e;
	if (expression != nullptr) expression->setExpression(e);
}

void Parameter::setReferenceParameter(Parameter* tp)
{
	if (tp == referenceParameter) return;

	if (referenceParameter != nullptr && !referenceParameter->isBeingDestroyed)
	{
		referenceParameter->removeParameterListener(this);
	}

	referenceParameter = tp;

	if (referenceParameter != nullptr)
	{
		parameterValueChanged(referenceParameter);
		referenceParameter->addParameterListener(this);
	}
}

void Parameter::setControlAutomation()
{
	jassertfalse;
}

var Parameter::getValue()
{
	GenericScopedLock lock(valueSetLock);
	return value;
}

var Parameter::getLerpValueTo(var targetValue, float weight)
{
	return getValue(); //to be overriden
}

void Parameter::setDefaultValue(var val, bool doResetValue)
{
	defaultValue = getCroppedValue(val);
	if (doResetValue) resetValue();
}

void Parameter::resetValue(bool silentSet)
{
	isOverriden = false;
	setValue(defaultValue, silentSet, false, false);
}

Array<UndoableAction*> Parameter::setUndoableValue(var newValue, bool onlyReturnAction, bool setSimilarSelected)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		setValue(newValue);
		return Array<UndoableAction*>();
	}

	if (lastUndoValue.isVoid()) lastUndoValue = newValue.clone();
	Array<UndoableAction*> actions;
	UndoableAction* a = new ParameterSetValueAction(this, lastUndoValue, newValue);
	actions.add(a);
	lastUndoValue = newValue.clone();

	if (setSimilarSelected && InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 1)
	{
		if (ControllableContainer* cc = getSelectedParentInHierarchy())
		{
			String relativeAddress = getControlAddress(cc);
			Array<ControllableContainer*> containers = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<ControllableContainer>();
			containers.removeAllInstancesOf(cc);

			for (ControllableContainer* c : containers)
			{
				if (Parameter* t = dynamic_cast<Parameter*>(c->getControllableForAddress(relativeAddress)))
				{
					actions.addArray(t->setUndoableValue(newValue, true, false));
				}
			}
		}
	}

	if (onlyReturnAction) return actions;

	UndoMaster::getInstance()->performActions("Set " + niceName + " value", actions);
	return actions;
}

void Parameter::setValue(juce::var _value, bool silentSet, bool force, bool forceOverride, bool setSimilarSelected)
{
	{
		GenericScopedLock lock(valueSetLock);

		var croppedValue = getCroppedValue(_value.clone());
		if (!alwaysNotify && !force && checkValueIsTheSame(value, croppedValue)) return;

		lastValue = value.clone();
		if (lastUndoValue.isVoid()) lastUndoValue = _value.clone();
		setValueInternal(croppedValue);
		if (!isOverriden /*&& !isControllableFeedbackOnly*/) isOverriden = croppedValue != defaultValue || forceOverride;
	}

	if (setSimilarSelected && InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 1)
	{
		if (ControllableContainer* cc = getSelectedParentInHierarchy())
		{
			String relativeAddress = getControlAddress(cc);
			Array<ControllableContainer*> containers = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<ControllableContainer>();
			containers.removeAllInstancesOf(cc);

			for (ControllableContainer* c : containers)
			{
				if (Parameter* t = dynamic_cast<Parameter*>(c->getControllableForAddress(relativeAddress)))
				{
					t->setValue(value, silentSet, force, forceOverride, false);
				}
			}
		}
	}

	if (!silentSet) notifyValueChanged();
}

void Parameter::resetLastUndoValue()
{
	lastUndoValue = getValue().clone();
}


bool Parameter::isComplex() const
{
	return value.isArray();
}

StringArray Parameter::getValuesNames()
{
	StringArray result;
	if (!isComplex()) result.add("Value");
	else
	{
		for (int i = 0; i < value.size(); ++i) result.add("Value " + String(i));
	}
	return result;
}

void Parameter::setRange(var min, var max)
{
	if (!canHaveRange) return;
	{
		GenericScopedLock lock(valueSetLock);
		if (isComplex())
		{
			if (!min.isArray() || min.size() != value.size()) return;
			if (!max.isArray() || max.size() != value.size()) return;
		}

		if (minimumValue == min && maximumValue == max) return;
		minimumValue = min;
		maximumValue = max;
	}

	WeakReference<Parameter> safeThis(this);
	notifyRangeChanged();
	auto* parameter = safeThis.get();
	if (parameter == nullptr || parameter->isBeingDestroyed) return;

	if (parameter->isOverriden) parameter->setValue(parameter->getValue());
	else parameter->resetValue();

	if (safeThis != nullptr && safeThis->isCustomizableByUser) safeThis->isOverriden = true;
}

void Parameter::clearRange()
{
	if (!canHaveRange) return;

	if (isComplex())
	{
		var minVal = var();
		var maxVal = var();

		{
			GenericScopedLock lock(valueSetLock);

			for (int i = 0; i < value.size(); ++i)
			{
				minVal.append(INT32_MIN);
				maxVal.append(INT32_MAX);
			}
		}

		setRange(minVal, maxVal);
	}
	else
	{
		setRange(INT32_MIN, INT32_MAX);
	}
}

bool Parameter::hasRange() const
{
	if (!canHaveRange) return false;
	{
		GenericScopedLock lock(valueSetLock);
		if (isComplex())
		{
			for (int i = 0; i < value.size(); ++i) if ((float)minimumValue[i] != INT32_MIN && (float)maximumValue[i] != INT32_MAX) return true;
		}
		else
		{
			if (minimumValue.isInt())
			{
				if ((int)minimumValue != INT32_MIN && (int)maximumValue != INT32_MAX) return true;
			}
			else if (minimumValue.isDouble())
			{
				if ((float)minimumValue != (float)INT32_MIN && (float)maximumValue != (float)INT32_MAX) return true;

			}
		}
	}

	return false;
}

var Parameter::getRange() const
{
	GenericScopedLock lock(valueSetLock);
	return var(Array<var>{ minimumValue, maximumValue });
}

void Parameter::setValueInternal(var& _value) //to override by child classes
{
	jassert(!value.isVoid());
	value = _value;

	jassert(checkVarIsConsistentWithType());
}

bool Parameter::checkValueIsTheSame(var newValue, var oldValue)
{
	if (oldValue.isArray())
	{
		if (!newValue.isArray()) return false;
		if (newValue.size() != oldValue.size()) return false;
		for (int i = 0; i < oldValue.size(); ++i) if (oldValue[i] != newValue[i]) return false;
		return true;
	}

	return newValue == oldValue;
}

bool Parameter::checkVarIsConsistentWithType() {
	if (type == Type::STRING)		return value.isString();
	else if (type == Type::INT)     return value.isBool() || value.isDouble() || value.isInt() || value.isInt64();
	else if (type == Type::BOOL)    return value.isBool() || value.isDouble() || value.isInt() || value.isInt64();
	else if (type == Type::FLOAT)   return value.isBool() || value.isDouble() || value.isInt() || value.isInt64();
	else if (type == Type::POINT2D || type == Type::POINT3D || type == Type::COLOR) return value.isArray();
	return true;
}

var Parameter::getCroppedValue(var originalValue)
{
	return originalValue;
}

void Parameter::setUndoableNormalizedValue(const var& newNormalizedValue, bool setSimilarSelected)
{
	if (!isComplex())
	{
		setUndoableValue(jmap<float>(newNormalizedValue, (float)minimumValue, (float)maximumValue), false, setSimilarSelected);
	}
	else
	{
		var newVal;
		for (int i = 0; i < value.size(); i++)
		{
			newVal.append(jmap<float>(newNormalizedValue[i], minimumValue[i], maximumValue[i]));
		}
		setUndoableValue(newVal, false, setSimilarSelected);
	}

}

void Parameter::setNormalizedValue(const var& normalizedValue, bool silentSet, bool force, bool setSimilarSelected)
{
	if (!isComplex())
	{
		setValue(jmap<float>(normalizedValue, (float)minimumValue, (float)maximumValue), silentSet, force, false, setSimilarSelected);

	}
	else
	{
		var targetVal;
		for (int i = 0; i < value.size(); i++) targetVal.append(jmap<float>(normalizedValue[i], minimumValue[i], maximumValue[i]));
		setValue(targetVal, silentSet, force, false, setSimilarSelected);
	}
}

var Parameter::getNormalizedValue(var forceVal) const
{
	var val = forceVal.isVoid() ? value : forceVal;

	if (type == BOOL) return (float)val;

	jassert(val.size() == value.size());

	if (!canHaveRange) return 0;

	if (!isComplex())
	{
		if ((float)minimumValue == (float)maximumValue) {
			return 0.0;
		}
		else
			return jmap<float>((float)val, (float)minimumValue, (float)maximumValue, 0.f, 1.f);
	}
	else
	{
		var normVal;
		for (int i = 0; i < val.size(); i++)
		{
			if ((float)minimumValue[i] == (float)maximumValue[i]) normVal.append(0.f);
			else normVal.append(jmap<float>((float)val[i], (float)minimumValue[i], (float)maximumValue[i], 0.f, 1.f));
		}

		return normVal;
	}
}

bool Parameter::setAttributeInternal(String param, var val)
{
	if (param == "alwaysNotify") alwaysNotify = val;
	else
	{
		return Controllable::setAttributeInternal(param, val);
	}

	return true;
}

StringArray Parameter::getValidAttributes() const
{
	StringArray att = Controllable::getValidAttributes();
	att.add("alwaysNotify");
	return att;
}

//helpers for fast typing

float Parameter::floatValue() { return (float)getValue(); }

double Parameter::doubleValue() { return (double)getValue(); }

int Parameter::intValue() { return (int)getValue(); }

bool Parameter::boolValue() { return (bool)getValue(); }

String Parameter::stringValue() {

	var val = getValue();

	if (!isComplex()) return val.toString();

	String s = val[0];
	for (int i = 1; i < val.size(); ++i) s += "," + val[i].toString();
	return "[" + s + "]";
}

void Parameter::notifyValueChanged() {

	//if(isNotifyingChange) return; //not sure about this, it may cause unsynced value changes
	//isNotifyingChange = true;

	const auto notificationLock = notifyLock;
	GenericScopedTryLock lock(*notificationLock);

	if (!lock.isLocked()) return;

	auto* mm = MessageManager::getInstanceWithoutCreating();
	if (mm != nullptr && mm->hasStopMessageBeenSent()) return;

	if (mm != nullptr && !mm->isThisTheMessageThread())
	{
		WeakReference<Parameter> safeThis(this);
		const auto valueCopy = getValue();

		getParameterValueDispatcher()->post([safeThis, valueCopy]()
			{
				auto* p = safeThis.get();
				if (p == nullptr || p->isBeingDestroyed) return;

				p->parameterListeners.callChecked(ParameterBailOutChecker{ safeThis }, &ParameterListener::parameterValueChangedWithValue, p, valueCopy);

				auto* pAfterListeners = safeThis.get();
				if (pAfterListeners == nullptr || pAfterListeners->isBeingDestroyed) return;

				pAfterListeners->queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::VALUE_CHANGED, pAfterListeners, valueCopy), false, true);
			});

		return;
	}

	WeakReference<Parameter> safeThis(this);
	const auto valueCopy = getValue();
	parameterListeners.callChecked(ParameterBailOutChecker{ safeThis }, &ParameterListener::parameterValueChangedWithValue, this, valueCopy);
	if (auto* p = safeThis.get(); p != nullptr && !p->isBeingDestroyed)
		p->queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::VALUE_CHANGED, p, valueCopy), false, true);
	//isNotifyingChange = false;
}

void Parameter::notifyRangeChanged()
{
	const auto arr = getRange();

	if (auto* mm = MessageManager::getInstanceWithoutCreating(); mm != nullptr && !mm->isThisTheMessageThread())
	{
		WeakReference<Parameter> safeThis(this);
		const auto safeRange = arr.clone();

		MessageManager::callAsync(
			[safeThis, safeRange]()
			{
				auto* p = safeThis.get();
				if (p == nullptr || p->isBeingDestroyed)
				{
					return;
				}

				p->parameterListeners.callChecked(ParameterBailOutChecker{ safeThis }, &ParameterListener::parameterRangeChanged, p);

				auto* pAfterListeners = safeThis.get();
				if (pAfterListeners == nullptr || pAfterListeners->isBeingDestroyed)
				{
					return;
				}

				pAfterListeners->queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::BOUNDS_CHANGED, pAfterListeners, safeRange));
			});
	}
	else
	{
		WeakReference<Parameter> safeThis(this);
		parameterListeners.callChecked(ParameterBailOutChecker{ safeThis }, &ParameterListener::parameterRangeChanged, this);
		if (auto* p = safeThis.get(); p != nullptr && !p->isBeingDestroyed)
		{
			p->queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::BOUNDS_CHANGED, p, arr));
		}
	}
}

void Parameter::expressionValueChanged(ScriptExpression*)
{
	setValue(expression->currentValue);
}

void Parameter::expressionStateChanged(ScriptExpression*)
{
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::EXPRESSION_STATE_CHANGED, this));
}

void Parameter::parameterValueChangedWithValue(Parameter* p, const var& value)
{
	// References must forward each captured value even when several source
	// changes have queued before their message-thread callbacks can run.
	if (p == referenceParameter) setValue(value);
	else parameterValueChanged(p);
}

void Parameter::parameterValueChanged(Parameter* p)
{
	if (p == referenceTarget.get())
	{
		setReferenceParameter(dynamic_cast<Parameter*>(referenceTarget->target.get()));
	}
	else if (p == referenceParameter)
	{
		setValue(referenceParameter->getValue());
	}
}

InspectableEditor* Parameter::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new ParameterEditor(this, isRoot);
}

ControllableDetectiveWatcher* Parameter::getDetectiveWatcher()
{
	return new ParameterDetectiveWatcher();
}

DashboardItem* Parameter::createDashboardItem()
{
	return new DashboardParameterItem(this);
}

bool Parameter::shouldBeSaved()
{
	if (Controllable::shouldBeSaved()) return true;
	if (forceSaveValue) return true;
	if (forceSaveRange) return true;
	if (isOverriden && !isControllableFeedbackOnly && isSavable) return true;
	if (controlMode != MANUAL) return true;
	if (colorStatusMap.size() > 0) return true;
	return false;
}

var Parameter::getJSONDataInternal()
{
	var data = Controllable::getJSONDataInternal();
	if (forceSaveValue || (isOverriden && !isControllableFeedbackOnly && isSavable)) data.getDynamicObject()->setProperty("value", value);

	if (controlMode != MANUAL)
	{
		data.getDynamicObject()->setProperty("controlMode", controlMode);
		if (controlMode == EXPRESSION) data.getDynamicObject()->setProperty("expression", controlExpression);
		else if (controlMode == AUTOMATION && automation != nullptr) data.getDynamicObject()->setProperty("paramAutomation", automation->getJSONData());
		else if (controlMode == REFERENCE && referenceTarget != nullptr) data.getDynamicObject()->setProperty("reference", referenceTarget->getJSONData());
	}

	if (alwaysNotify) data.getDynamicObject()->setProperty("alwaysNotify", true);


	if (hasRange() && (!saveValueOnly || forceSaveRange || isCustomizableByUser))
	{
		if ((int)minimumValue != INT32_MIN) data.getDynamicObject()->setProperty("minValue", minimumValue);
		if ((int)maximumValue != INT32_MAX) data.getDynamicObject()->setProperty("maxValue", maximumValue);
		data.getDynamicObject()->setProperty("default", defaultValue);
	}

	if (colorStatusMap.size() > 0)
	{
		var cMapData;
		HashMap<var, Colour>::Iterator it(colorStatusMap);
		while (it.next())
		{
			cMapData.append(it.getKey());
			var colorVar;
			Colour c = it.getValue();
			colorVar.append(c.getFloatRed());
			colorVar.append(c.getFloatGreen());
			colorVar.append(c.getFloatBlue());
			colorVar.append(c.getFloatAlpha());
			cMapData.append(colorVar);
		}

		data.getDynamicObject()->setProperty("colorStatusMap", cMapData);
	}

	return data;
}

void Parameter::loadJSONDataInternal(var data)
{
	Controllable::loadJSONDataInternal(data);

	if (!saveValueOnly || forceSaveRange || isCustomizableByUser)
	{
		setRange(data.getProperty("minValue", minimumValue), data.getProperty("maxValue", maximumValue));
		defaultValue = data.getProperty("default", defaultValue);
	}

	if (data.getDynamicObject()->hasProperty("value")) setValue(data.getProperty("value", defaultValue), false, false, true);
	else value = defaultValue;

	if (data.getDynamicObject()->hasProperty("controlMode")) setControlMode((ControlMode)(int)data.getProperty("controlMode", MANUAL));

	if (data.getDynamicObject()->hasProperty("expression")) setControlExpression(data.getProperty("expression", ""));
	else if (data.getDynamicObject()->hasProperty("paramAutomation") && automation != nullptr) automation->loadJSONData(data.getProperty("paramAutomation", var()));
	else if (data.getDynamicObject()->hasProperty("reference") && referenceTarget != nullptr) referenceTarget->loadJSONData(data.getProperty("reference", var()));

	var cMapData = data.getProperty("colorStatusMap", var());
	for (int i = 0; i < cMapData.size(); i += 2)
	{
		var cData = cMapData[i + 1];
		Colour c = Colour::fromFloatRGBA((float)cData[0], (float)cData[1], (float)cData[2], (float)cData[3]);
		colorStatusMap.set(cMapData[i], c);
	}

	alwaysNotify = data.getProperty("alwaysNotify", alwaysNotify);

	isOverriden = true;
}

void Parameter::setupFromJSONData(var data)
{
	Controllable::setupFromJSONData(data);

	if (data.hasProperty("min") || data.hasProperty("max")) setRange(data.getProperty("min", INT32_MIN), data.getProperty("max", INT32_MAX));

	if (data.hasProperty("default"))
	{
		defaultValue = data.getProperty("default", defaultValue);
		setValue(data.getProperty("default", defaultValue));
	}
}

void Parameter::getRemoteControlDataInternal(var& data)
{
	Controllable::getRemoteControlDataInternal(data);

	var v = getRemoteControlValue();
	var valueData = var();
	if (v.isArray()) valueData = v;
	else valueData.append(v); //always need to be an array
	if (!valueData.isVoid()) data.getDynamicObject()->setProperty("VALUE", valueData);

	var rangeData = getRemoteControlRange();
	if (!rangeData.isVoid()) data.getDynamicObject()->setProperty("RANGE", rangeData);

	var extType = var();
	extType.append(getTypeString());
	data.getDynamicObject()->setProperty("EXTENDED_TYPE", extType);
}

var Parameter::getRemoteControlValue()
{
	return getValue();
}

var Parameter::getRemoteControlRange()
{
	if (!hasRange()) return var();
	var range = var();

	if (isComplex())
	{
		for (int i = 0; i < minimumValue.size(); i++)
		{
			var rData(new DynamicObject());
			rData.getDynamicObject()->setProperty("MIN", minimumValue[i]);
			rData.getDynamicObject()->setProperty("MAX", maximumValue[i]);
			range.append(rData);
		}
	}
	else
	{
		var rData(new DynamicObject());
		rData.getDynamicObject()->setProperty("MIN", minimumValue);
		rData.getDynamicObject()->setProperty("MAX", maximumValue);
		range.append(rData);
	}

	return range;
}

var Parameter::getValueFromScript(const juce::var::NativeFunctionArgs& a)
{
	Parameter* p = getObjectFromJS<Parameter>(a);
	if (p == nullptr) return var();
	WeakReference<Parameter> pRef(p);
	if (pRef == nullptr || pRef.wasObjectDeleted()) return var();
	return p->getValue();
}

var Parameter::resetValueFromScript(const juce::var::NativeFunctionArgs& a)
{
	Parameter* p = getObjectFromJS<Parameter>(a);
	if (p == nullptr) return var();
	WeakReference<Parameter> pRef(p);
	if (pRef == nullptr || pRef.wasObjectDeleted()) return var();

	pRef->resetValue();

	return var();
}

var Parameter::getRangeFromScript(const juce::var::NativeFunctionArgs& a)
{
	Parameter* p = getObjectFromJS<Parameter>(a);
	if (p == nullptr) return var();
	WeakReference<Parameter> pRef(p);
	if (pRef == nullptr || pRef.wasObjectDeleted()) return var();
	var result;
	result.append(p->minimumValue);
	result.append(p->maximumValue);
	return result;
}

var Parameter::setRangeFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (a.numArguments < 2) return var();
	Parameter* p = getObjectFromJS<Parameter>(a);
	if (p == nullptr) return var();
	WeakReference<Parameter> pRef(p);
	if (pRef == nullptr || pRef.wasObjectDeleted()) return var();
	if (!p->canHaveRange || !p->isCustomizableByUser)
	{
		NLOGWARNING(p->niceName, "This parameter's range can not be changed.");
		return var();
	}

	var newMin = a.arguments[0];
	var newMax = a.arguments[1];
	p->setRange(newMin, newMax);

	return var();
}

var Parameter::hasRangeFromScript(const juce::var::NativeFunctionArgs& a)
{
	Parameter* p = getObjectFromJS<Parameter>(a);
	if (p == nullptr) return var();
	WeakReference<Parameter> pRef(p);
	if (pRef == nullptr || pRef.wasObjectDeleted()) return var();
	return p->hasRange();
}

String Parameter::getScriptTargetString()
{
	return  "[" + niceName + " : " + getTypeString() + " > " + stringValue() + "]";
}


// UNDO MANAGEMENT

Parameter* Parameter::ParameterAction::getParameter() {
	return dynamic_cast<Parameter*>(getControllable());
}

bool Parameter::ParameterSetValueAction::perform()
{
	Parameter* p = getParameter();
	if (p == nullptr)
	{
		LOGWARNING("Undo set value : parameter not found " << controlAddress);
		return false;
	}

	p->setValue(newValue);
	return true;
}

bool Parameter::ParameterSetValueAction::undo()
{
	Parameter* p = getParameter();
	if (p == nullptr)
	{
		LOGWARNING("Undo set value : parameter not found " << controlAddress);
		return false;
	}

	p->setValue(oldValue);
	p->lastUndoValue = oldValue;
	return true;
}


//Value Interpolator

Parameter::ValueInterpolator::ValueInterpolator(WeakReference<Parameter> p, var targetValue, float time, Automation* a) :
	parameter(p),
	targetValue(targetValue),
	time(time),
	timeAtStart(Time::getMillisecondCounterHiRes() / 1000.0),
	valueAtStart(parameter->getValue()),
	automation(a)
{
	parameter->addInspectableListener(this);
}

Parameter::ValueInterpolator::~ValueInterpolator()
{
	if (parameter != nullptr && !parameter.wasObjectDeleted()) parameter->removeInspectableListener(this);

	masterReference.clear();
}

bool Parameter::ValueInterpolator::update()
{
	if (parameter == nullptr || parameter.wasObjectDeleted()) return false;

	double t = Time::getMillisecondCounterHiRes() / 1000.0;
	double relT = (t - timeAtStart) / time;

	if (relT >= 1)
	{
		parameter->setValue(targetValue);
		return false;
	}
	else
	{
		var tVal;

		float pos = automation->getValueAtPosition(relT);

		if (targetValue.isArray())
		{
			for (int i = 0; i < targetValue.size(); i++)
			{
				tVal.append(jmap<float>(pos, (float)valueAtStart[i], (float)targetValue[i]));
			}
		}
		else
		{
			tVal = jmap<float>(pos, (float)valueAtStart, (float)targetValue);
		}

		parameter->setValue(tVal);
	}

	return true;
}

void Parameter::ValueInterpolator::inspectableDestroyed(Inspectable* i)
{
	if (i == parameter || parameter.wasObjectDeleted())
	{
		parameter = nullptr;
	}
}

Parameter::ValueInterpolator::Manager::Manager() :
	Thread("Value Interpolator")
{
	startThread();
}

Parameter::ValueInterpolator::Manager::~Manager()
{
	stopThread(1000);
}


void Parameter::ValueInterpolator::updateParams(var newTargetValue, float newTime, Automation* newAutomation)
{
	//	GenericScopedLock lock(updateLock);
	targetValue = newTargetValue;
	time = newTime;
	automation = newAutomation;

	valueAtStart = parameter->getValue();
	timeAtStart = Time::getMillisecondCounterHiRes() / 1000.0;
}

void Parameter::ValueInterpolator::Manager::interpolate(WeakReference<Parameter> p, var targetValue, float time, Automation* a)
{
	jassert(p->getValue().size() == targetValue.size());
	GenericScopedLock lock(interpLock);

	WeakReference<ValueInterpolator> interp = getInterpolationWith(p);
	if (interp != nullptr && !interp.wasObjectDeleted())
	{
		interp->updateParams(targetValue, time, a);
		return;
	}

	interp = new ValueInterpolator(p, targetValue, time, a);

	if (interp == nullptr) return;

	//MessageManagerLock mmLock;
	if (interp.wasObjectDeleted()) return;
	interpolators.add(interp);
	interpolatorMap.set(p, interp);
}

WeakReference<Parameter::ValueInterpolator> Parameter::ValueInterpolator::Manager::getInterpolationWith(Parameter* p)
{
	if (interpolatorMap.contains(p)) return interpolatorMap[p];
	return nullptr;
}

void Parameter::ValueInterpolator::Manager::removeInterpolationWith(Parameter* p)
{
	GenericScopedLock lock(interpLock);
	if (interpolatorMap.contains(p))
	{
		WeakReference<ValueInterpolator> interp = interpolatorMap[p];
		if (interp.wasObjectDeleted()) return;
		interpolatorMap.remove(p);
		interpolators.removeObject(interp);
	}
}


void Parameter::ValueInterpolator::Manager::run()
{
	const int targetSleepMS = 20; //50fps

	Array<WeakReference<ValueInterpolator>> interpToRemove;


	while (!threadShouldExit())
	{
		int timeToWait = targetSleepMS;

		{
			GenericScopedLock lock(interpLock);
			double t = Time::getMillisecondCounter();
			for (auto& i : interpolators)
			{
				if (i == nullptr) continue;
				bool result = i->update();
				if (!result) interpToRemove.add(i);
			}


			for (auto& i : interpToRemove)
			{
				interpolatorMap.removeValue(i);
				interpolators.removeObject(i);
			}

			interpToRemove.clear();

			double diff = Time::getMillisecondCounter() - t;
			timeToWait = targetSleepMS - diff;
		}

		if (timeToWait > 0) wait(timeToWait);
	}
}