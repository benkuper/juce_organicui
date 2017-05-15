#include "Parameter.h"
/*
  ==============================================================================

	Parameter.cpp
	Created: 8 Mar 2016 1:08:19pm
	Author:  bkupe

  ==============================================================================
*/


Parameter::Parameter(const Type &type, const String &niceName, const String &description, var initialValue, var minValue = var(), var maxValue = var(), bool enabled) :
	Controllable(type, niceName, description, enabled),
	lockManualControlMode(false),
	controlMode(MANUAL),
	isPresettable(true),
	isOverriden(false),
	autoAdaptRange(false),
	queuedNotifier(100)
{
	minimumValue = minValue;
	maximumValue = maxValue;
	defaultMinValue = minValue;
	defaultMaxValue = maxValue;

	defaultValue = initialValue;

	resetValue(true);

	scriptObject.setMethod("get", Parameter::getValueFromScript);
	scriptObject.setMethod("set", Controllable::setValueFromScript);
}

Parameter::~Parameter() {
	Parameter::masterReference.clear();
}


void Parameter::setControlMode(ControlMode _mode)
{
	if (_mode == controlMode) return;

	controlMode = _mode;

	switch (controlMode)
	{
	case MANUAL:
		expression = nullptr;
		break;

	case REFERENCE:
		expression = nullptr;
		break;

	case EXPRESSION:
		expression = new ScriptExpression();
		expression->setExpression(controlExpression);
		expression->addExpressionListener(this);
		break;
	}

	listeners.call(&Listener::parameterControlModeChanged, this);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::CONTROLMODE_CHANGED, this));
}

void Parameter::setControlExpression(const String & e)
{
	controlExpression = e;
	if (expression != nullptr) expression->setExpression(e);
}

var Parameter::getValue()
{
	return value;
}

void Parameter::resetValue(bool silentSet)
{
	isOverriden = false;
	setValue(defaultValue, silentSet, true);
}

void Parameter::setValue(var _value, bool silentSet, bool force)
{

	if (!force && checkValueIsTheSame(_value, value)) return;
	lastValue = var(value);
	setValueInternal(_value);

	if (_value != defaultValue) isOverriden = true;

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

	if (setDefaultRange)
	{
		defaultMinValue = min;
		defaultMaxValue = max;
	}

	minimumValue = min;
	maximumValue = max;


	listeners.call(&Listener::parameterRangeChanged, this);
	var arr;
	arr.append(minimumValue); arr.append(maximumValue);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::BOUNDS_CHANGED, this, arr));

	setValue(value); //if value is outside range, this will change the value
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

void Parameter::setNormalizedValue(const float & normalizedValue, bool silentSet, bool force)
{
	setValue(jmap<float>(normalizedValue, (float)minimumValue, (float)maximumValue), silentSet, force);
}

float Parameter::getNormalizedValue()
{
	if (type != FLOAT) return 0;
	if (minimumValue == maximumValue) {
		return 0.0;
	} else
		return jmap<float>((float)value, (float)minimumValue, (float)maximumValue, 0.f, 1.f);
}

void Parameter::notifyValueChanged() {
	listeners.call(&Listener::parameterValueChanged, this);
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::VALUE_CHANGED,this,getValue()));
}

void Parameter::expressionValueChanged(ScriptExpression *)
{
	setValue(expression->currentValue);
}

void Parameter::expressionStateChanged(ScriptExpression *)
{
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::EXPRESSION_STATE_CHANGED, this));
}

InspectableEditor * Parameter::getEditor(bool isRoot)
{
	return new ParameterEditor(this, isRoot);
}

var Parameter::getJSONDataInternal()
{
	var data = Controllable::getJSONDataInternal();
	data.getDynamicObject()->setProperty("value", value);
	if (saveValueOnly) return data;
	data.getDynamicObject()->setProperty("minValue", minimumValue);
	data.getDynamicObject()->setProperty("maxValue", maximumValue);
	data.getDynamicObject()->setProperty("controlMode", controlMode);
	if(controlMode == EXPRESSION) data.getDynamicObject()->setProperty("expression", controlExpression);
	return data;
}

void Parameter::loadJSONDataInternal(var data)
{
	Controllable::loadJSONDataInternal(data);

	if (!saveValueOnly) setRange(data.getProperty("minValue", minimumValue), data.getProperty("maxValue", maximumValue));
	if (data.getDynamicObject()->hasProperty("value")) setValue(data.getProperty("value", 0));

	if (data.getDynamicObject()->hasProperty("controlMode")) controlMode = (ControlMode)(int)data.getProperty("controlMode", MANUAL);
	if (data.getDynamicObject()->hasProperty("expression")) controlExpression = data.getProperty("expression", "");

}

var Parameter::getValueFromScript(const juce::var::NativeFunctionArgs & a)
{
	Parameter * c = getObjectFromJS<Parameter>(a);
	if (c == nullptr) return var();
	return c->getValue();
}


//JS Helper
