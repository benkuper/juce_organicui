#include "Parameter.h"
/*
  ==============================================================================

	Parameter.cpp
	Created: 8 Mar 2016 1:08:19pm
	Author:  bkupe

  ==============================================================================
*/

Parameter::Parameter(const Type &type, const String &niceName, const String &description, var initialValue, var minValue, var maxValue, bool enabled) :
	Controllable(type, niceName, description, enabled),
	defaultValue(initialValue),
	canHaveRange(false),
	minimumValue(minValue),
	maximumValue(maxValue),
	defaultMinValue(minValue),
	defaultMaxValue(maxValue),
    lockManualControlMode(false),
    controlMode(MANUAL),
	canBeAutomated(false),
    referenceParameter(nullptr),
    isPresettable(true),
    isOverriden(false),
    autoAdaptRange(false),
    forceSaveValue(false),
	queuedNotifier(100)
{
	resetValue(true);

	scriptObject.setMethod("get", Parameter::getValueFromScript);
	scriptObject.setMethod("set", Controllable::setValueFromScript);
}

Parameter::~Parameter() {
	Parameter::masterReference.clear();
	setReferenceParameter(nullptr);
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

void Parameter::setControlExpression(const String & e)
{
	controlExpression = e;
	if (expression != nullptr) expression->setExpression(e);
}

void Parameter::setReferenceParameter(Parameter * tp)
{
	if (tp == referenceParameter) return;
	
	if(referenceParameter != nullptr)
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
	return value;
}

var Parameter::getLerpValueTo(var targetValue, float weight)
{
	return value; //to be overriden
}

void Parameter::resetValue(bool silentSet)
{
	isOverriden = false;
	setValue(defaultValue, silentSet, true, false);
}

UndoableAction * Parameter::setUndoableValue(var oldValue, var newValue, bool onlyReturnAction)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		//if Main Engine loading, just set the value without undo history
		setValue(newValue);
		return nullptr;
	}

	UndoableAction * a = new ParameterSetValueAction(this, oldValue, newValue);
	if (onlyReturnAction) return a;

	UndoMaster::getInstance()->performAction("Set " + niceName + " value", a);
	return a;
}

void Parameter::setValue(var _value, bool silentSet, bool force, bool forceOverride)
{
	if (!force && checkValueIsTheSame(_value, value)) return;
	
	valueSetLock.enter();
	lastValue = var(value);
	valueSetLock.exit();

	setValueInternal(_value);

	isOverriden =  _value != defaultValue || forceOverride;

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
		for (int i = 0; i < value.size(); i++) result.add("Value " + String(i));
	}
	return result;
}

void Parameter::setRange(var min, var max, bool setDefaultRange) {

	if(isComplex() && (!(min.isArray() && min.size() == value.size()) || !(max.isArray() && max.size() == value.size()))) return;

	if (setDefaultRange)
	{
		defaultMinValue = min;
		defaultMaxValue = max;
	}

	if (minimumValue == min && maximumValue == max) return;

	minimumValue = min;
	maximumValue = max;

	listeners.call(&ParameterListener::parameterRangeChanged, this);
	var arr;
	arr.append(minimumValue); arr.append(maximumValue);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::BOUNDS_CHANGED, this, arr));

	if (isOverriden) setValue(value); //if value is outside range, this will change the value
	else resetValue();
}

void Parameter::clearRange()
{
	setRange(INT32_MIN, INT32_MAX);
}

bool Parameter::hasRange()
{
	return canHaveRange;
}

void Parameter::setValueInternal(var & _value) //to override by child classes
{
	value = _value;

#ifdef JUCE_DEBUG
	if (!checkVarIsConsistentWithType()) DBG("Problem with var type");
#endif
}

bool Parameter::checkValueIsTheSame(var newValue, var oldValue)
{
	if (oldValue.isArray())
	{
		if (!newValue.isArray()) return false;
		if (newValue.size() != oldValue.size()) return false;
		for (int i = 0; i < oldValue.size(); i++) if (oldValue[i] != newValue[i]) return false;
		return true;
	}

	return newValue == oldValue;
}

bool Parameter::checkVarIsConsistentWithType() {
	if (type == Type::STRING)  return value.isString();
	else if (type == Type::INT)     return value.isInt();
	else if (type == Type::BOOL)    return value.isBool();
	else if (type == Type::FLOAT)   return value.isDouble();
	return true;
}

void Parameter::setUndoableNormalizedValue(const float & oldNormalizedValue, const float & newNormalizedValue)
{
	setUndoableValue(jmap<float>(oldNormalizedValue, (float)minimumValue, (float)maximumValue), jmap<float>(newNormalizedValue, (float)minimumValue, (float)maximumValue));
}

void Parameter::setNormalizedValue(const float & normalizedValue, bool silentSet, bool force)
{
	setValue(jmap<float>(normalizedValue, (float)minimumValue, (float)maximumValue), silentSet, force);
}

float Parameter::getNormalizedValue()
{
	if (type == BOOL) return (float)value;

	if (!canHaveRange) return 0;

	if ((float)minimumValue == (float)maximumValue) {
		return 0.0;
	} else
		return jmap<float>((float)value, (float)minimumValue, (float)maximumValue, 0.f, 1.f);
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
	for (int i = 1; i < val.size(); i++) s += "," + val[i].toString();
	return "[" + s + "]";
}

void Parameter::notifyValueChanged() {
	listeners.call(&ParameterListener::parameterValueChanged, this);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::VALUE_CHANGED,this, getValue()));
}

void Parameter::expressionValueChanged(ScriptExpression *)
{
	setValue(expression->currentValue);
}

void Parameter::expressionStateChanged(ScriptExpression *)
{
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::EXPRESSION_STATE_CHANGED, this));
}

void Parameter::parameterValueChanged(Parameter * p)
{
	if (p == referenceTarget.get())
	{
		setReferenceParameter(dynamic_cast<Parameter *>(referenceTarget->target.get()));
	} else if (p == referenceParameter)
	{
		setValue(referenceParameter->getValue());
	}
}

InspectableEditor * Parameter::getEditor(bool isRoot)
{
	return new ParameterEditor(this, isRoot);
}

DashboardItem* Parameter::createDashboardItem()
{
	return new DashboardParameterItem(this);
}

var Parameter::getJSONDataInternal()
{
	var data = Controllable::getJSONDataInternal();
	data.getDynamicObject()->setProperty("value", value);
	
	if (controlMode != MANUAL)
	{
		data.getDynamicObject()->setProperty("controlMode", controlMode);
		if (controlMode == EXPRESSION) data.getDynamicObject()->setProperty("expression", controlExpression);
		else if (controlMode == AUTOMATION && automation != nullptr) data.getDynamicObject()->setProperty("paramAutomation", automation->getJSONData());
		else if (controlMode == REFERENCE && referenceTarget != nullptr) data.getDynamicObject()->setProperty("reference", referenceTarget->getJSONData());
	}

	if (saveValueOnly) return data;

	if (hasRange())
	{
		if((int)minimumValue != INT32_MIN) data.getDynamicObject()->setProperty("minValue", minimumValue);
		if((int)maximumValue != INT32_MAX) data.getDynamicObject()->setProperty("maxValue", maximumValue);
	}
	return data;
}

void Parameter::loadJSONDataInternal(var data)
{
	Controllable::loadJSONDataInternal(data);

	if (!saveValueOnly) setRange(data.getProperty("minValue", minimumValue), data.getProperty("maxValue", maximumValue));
	if (data.getDynamicObject()->hasProperty("value")) setValue(data.getProperty("value", 0),false, true, true);

	if (data.getDynamicObject()->hasProperty("controlMode")) setControlMode((ControlMode)(int)data.getProperty("controlMode", MANUAL));
	
	if (data.getDynamicObject()->hasProperty("expression")) setControlExpression(data.getProperty("expression", ""));
	else if (data.getDynamicObject()->hasProperty("paramAutomation") && automation != nullptr) automation->loadJSONData(data.getProperty("paramAutomation", var()));
	else if (data.getDynamicObject()->hasProperty("reference") && referenceTarget != nullptr) referenceTarget->loadJSONData(data.getProperty("reference", var()));

 	if (data.getDynamicObject()->hasProperty("editable")) setControllableFeedbackOnly(!data.getProperty("editable", true));
}

var Parameter::getValueFromScript(const juce::var::NativeFunctionArgs & a)
{
	WeakReference<Parameter> c = getObjectFromJS<Parameter>(a);
	if (c == nullptr || c.wasObjectDeleted()) return var();
	return c->getValue();
}


// UNDO MANAGEMENT

Parameter * Parameter::ParameterAction::getParameter() {
	return dynamic_cast<Parameter *>(getControllable());
}

bool Parameter::ParameterSetValueAction::perform()
{
	Parameter * p = getParameter();
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
	Parameter * p = getParameter();
	if (p == nullptr)
	{
		LOGWARNING("Undo set value : parameter not found " << controlAddress);
		return false;
	}

	p->setValue(oldValue);
	return true;
}
