/*
  ==============================================================================

	Parameter.cpp
	Created: 8 Mar 2016 1:08:19pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

juce_ImplementSingleton(Parameter::ValueInterpolator::Manager);


Parameter::Parameter(const Type& type, const String& niceName, const String& description, var initialValue, var minValue, var maxValue, bool enabled) :
	Controllable(type, niceName, description, enabled),
	defaultValue(initialValue),
	value(initialValue),
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
	queuedNotifier(100)
{

	scriptObject.setMethod("get", Parameter::getValueFromScript);
	scriptObject.setMethod("set", Controllable::setValueFromScript);
	scriptObject.setMethod("resetValue", Parameter::resetValueFromScript);
	scriptObject.setMethod("getRange", Parameter::getRangeFromScript);
	scriptObject.setMethod("setRange", Parameter::setRangeFromScript);
	scriptObject.setMethod("hasRange", Parameter::hasRangeFromScript);
}

Parameter::~Parameter()
{
	if (referenceTarget != nullptr) referenceTarget->removeParameterListener(this); //avoid reassigning on deletion
	setReferenceParameter(nullptr);

	if (queuedNotifier.isUpdatePending())
	{
		MessageManagerLock mmLock(Thread::getCurrentThread());
		if (mmLock.lockWasGained()) queuedNotifier.handleUpdateNowIfNeeded();
		queuedNotifier.cancelPendingUpdate();
	}

	Parameter::masterReference.clear();
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
		expression.reset(new ScriptExpression());
		expression->setExpression(controlExpression);
		expression->addExpressionListener(this);
		break;

	case AUTOMATION:
		setControlAutomation();
		break;

	}

	listeners.call(&ParameterListener::parameterControlModeChanged, this);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::CONTROLMODE_CHANGED, this));
}

void Parameter::setControlExpression(const String& e)
{
	controlExpression = e;
	if (expression != nullptr) expression->setExpression(e);
}

void Parameter::setReferenceParameter(Parameter* tp)
{
	if (tp == referenceParameter) return;

	if (referenceParameter != nullptr)
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
	GenericScopedLock<SpinLock> lock(valueSetLock);
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
	setValue(defaultValue, silentSet, true, false);
}

UndoableAction* Parameter::setUndoableValue(var oldValue, var newValue, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		setValue(newValue);
		return nullptr;
	}

	UndoableAction* a = new ParameterSetValueAction(this, oldValue, newValue);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Set " + niceName + " value", a);
	return a;
}

void Parameter::setValue(var _value, bool silentSet, bool force, bool forceOverride)
{
	{
		GenericScopedLock<SpinLock> lock(valueSetLock);

		var croppedValue = getCroppedValue(_value.clone());

		if (!alwaysNotify && !force && checkValueIsTheSame(value, croppedValue)) return;

		lastValue = var(value.clone());
		setValueInternal(croppedValue);
		if (!isOverriden /*&& !isControllableFeedbackOnly*/) isOverriden = croppedValue != defaultValue || forceOverride;
	}
	if (!silentSet) notifyValueChanged();
}


bool Parameter::isComplex()
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
		GenericScopedLock<SpinLock> lock(valueSetLock);
		if (isComplex())
		{
			if (!min.isArray() || min.size() != value.size()) return;
			if (!max.isArray() || max.size() != value.size()) return;
		}

		if (minimumValue == min && maximumValue == max) return;
		minimumValue = min;
		maximumValue = max;
	}

	listeners.call(&ParameterListener::parameterRangeChanged, this);
	var arr;
	arr.append(minimumValue); arr.append(maximumValue);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::BOUNDS_CHANGED, this, arr));

	if (isOverriden) setValue(value); //if value is outside range, this will change the value
	else resetValue();

	if (isCustomizableByUser) isOverriden = true;
}

void Parameter::clearRange()
{
	if (!canHaveRange) return;

	if (isComplex())
	{
		{
			GenericScopedLock<SpinLock> lock(valueSetLock);
			var minVal = var();
			var maxVal = var();
			for (int i = 0; i < value.size(); ++i)
			{
				minVal.append(INT32_MIN);
				maxVal.append(INT32_MAX);
			}
			minimumValue = minVal;
			maximumValue = maxVal;
		}

	}
	else
	{
		setRange(INT32_MIN, INT32_MAX);
	}
}

bool Parameter::hasRange()
{
	if (!canHaveRange) return false;
	{
		GenericScopedLock<SpinLock> lock(valueSetLock);
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
				if ((float)minimumValue != INT32_MIN && (float)maximumValue != INT32_MAX) return true;

			}
		}
	}

	return false;
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

void Parameter::setUndoableNormalizedValue(const var& oldNormalizedValue, const var& newNormalizedValue)
{
	if (!isComplex())
	{
		setUndoableValue(jmap<float>(oldNormalizedValue, (float)minimumValue, (float)maximumValue), jmap<float>(newNormalizedValue, (float)minimumValue, (float)maximumValue));
	}
	else
	{
		var oldVal;
		var newVal;
		for (int i = 0; i < value.size(); i++)
		{
			oldVal.append(jmap<float>(oldNormalizedValue[i], minimumValue[i], maximumValue[i]));
			newVal.append(jmap<float>(oldNormalizedValue[i], minimumValue[i], maximumValue[i]));
		}
		setUndoableValue(oldVal, newVal);
	}

}

void Parameter::setNormalizedValue(const var& normalizedValue, bool silentSet, bool force)
{
	if (!isComplex())
	{
		setValue(jmap<float>(normalizedValue, (float)minimumValue, (float)maximumValue), silentSet, force);

	}
	else
	{
		var targetVal;
		for (int i = 0; i < value.size(); i++) targetVal.append(jmap<float>(normalizedValue[i], minimumValue[i], maximumValue[i]));
		setValue(targetVal, silentSet, force);
	}
}

var Parameter::getNormalizedValue()
{
	if (type == BOOL) return (float)value;

	if (!canHaveRange) return 0;

	if (!isComplex())
	{
		if ((float)minimumValue == (float)maximumValue) {
			return 0.0;
		}
		else
			return jmap<float>((float)value, (float)minimumValue, (float)maximumValue, 0.f, 1.f);
	}
	else
	{
		var normVal;
		for (int i = 0; i < value.size(); i++)
		{
			if ((float)minimumValue[i] == (float)maximumValue[i]) normVal.append(0.f);
			else normVal.append(jmap<float>((float)value[i], (float)minimumValue[i], (float)maximumValue[i], 0.f, 1.f));
		}

		return normVal;
	}
}

void Parameter::setAttribute(String param, var val)
{
	Controllable::setAttribute(param, val);

	if (param == "alwaysNotify") alwaysNotify = val;
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
	listeners.call(&ParameterListener::parameterValueChanged, this);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::VALUE_CHANGED, this, getValue()));
}

void Parameter::expressionValueChanged(ScriptExpression*)
{
	setValue(expression->currentValue);
}

void Parameter::expressionStateChanged(ScriptExpression*)
{
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::EXPRESSION_STATE_CHANGED, this));
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
	return true;
}


//Value Interpolator

Parameter::ValueInterpolator::ValueInterpolator(WeakReference<Parameter> p, var targetValue, float time, Automation* a, float frequency) :
	Thread("Value Interpolator"),
	parameter(p),
	targetValue(targetValue),
	time(time),
	timeAtStart(0),
	automation(a),
	interpolatorNotifier(1)
{

	sleepMS = 1000.0 / frequency;
	startThread();
}

Parameter::ValueInterpolator::~ValueInterpolator()
{
	if (Thread::getCurrentThreadId() != getThreadId()) stopThread(100);

	if (interpolatorNotifier.isUpdatePending())
	{
		MessageManagerLock mmLock(Thread::getCurrentThread());
		if (mmLock.lockWasGained()) interpolatorNotifier.handleUpdateNowIfNeeded();
		interpolatorNotifier.cancelPendingUpdate();
	}

	masterReference.clear();
}

void Parameter::ValueInterpolator::run()
{
	timeAtStart = Time::getMillisecondCounterHiRes() / 1000.0;
	valueAtStart = parameter->getValue();

	jassert(valueAtStart.size() == targetValue.size());

	while (!threadShouldExit() && !parameter.wasObjectDeleted())
	{

		GenericScopedLock lock(updateLock);

		double t = Time::getMillisecondCounterHiRes() / 1000.0;
		double relT = (t - timeAtStart) / time;

		if (relT >= 1)
		{
			parameter->setValue(targetValue);
			listeners.call(&InterpolatorListener::interpolationFinished, this);
			interpolatorNotifier.addMessage(new InterpolatorEvent(InterpolatorEvent::INTERPOLATION_FINISHED, this));
			return;
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

		sleep(sleepMS);
	}
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

	//removeInterpolationWith(p);
	//GenericScopedLock lock(interpLock);

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
	interp->addAsyncInterpolatorListener(this);


	interpolators.add(interp);
	interpolatorMap.set(p, interp);
}


void Parameter::ValueInterpolator::Manager::interpolationFinished(WeakReference<ValueInterpolator> source)
{
	//not used now, using async to avoid same thread deletion
	if (source.wasObjectDeleted()) return;
	if (source->parameter != nullptr && !source->parameter.wasObjectDeleted()) removeInterpolationWith(source->parameter);
	else
	{
		source->removeInterpolatorListener(this);
		interpolatorMap.removeValue(source);
		interpolators.removeObject(source);
	}
}

void Parameter::ValueInterpolator::Manager::newMessage(const ValueInterpolator::InterpolatorEvent& e)
{
	e.item->removeInterpolatorListener(this);

	if (interpolatorMap.contains(e.item->parameter))
	{
		//GenericScopedLock lock(interpLock);
		WeakReference<ValueInterpolator> interp = interpolatorMap[e.item->parameter];
		if (interp.wasObjectDeleted()) return;
		interpolatorMap.remove(e.item->parameter);
		interpolators.removeObject(interp);
	}
	else
	{
		interpolatorMap.removeValue(e.item);
		interpolators.removeObject(e.item);
	}
}

WeakReference<Parameter::ValueInterpolator> Parameter::ValueInterpolator::Manager::getInterpolationWith(Parameter* p)
{
	if (interpolatorMap.contains(p)) return interpolatorMap[p];
	return nullptr;
}

void Parameter::ValueInterpolator::Manager::removeInterpolationWith(Parameter* p)
{
	if (interpolatorMap.contains(p))
	{
		GenericScopedLock lock(interpLock);
		WeakReference<ValueInterpolator> interp = interpolatorMap[p];
		if (interp.wasObjectDeleted()) return;
		interpolatorMap.remove(p);
		interpolators.removeObject(interp);
	}
}