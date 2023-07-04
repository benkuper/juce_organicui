/*
  ==============================================================================

	Point2DParameter.cpp
	Created: 30 Sep 2016 9:37:36am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

Point2DParameter::Point2DParameter(const String& niceName, const String& description, bool _enabled) :
	Parameter(POINT2D, niceName, description, 0, 0, 1, _enabled),
	x(0), y(0),
	defaultUI(FloatParameter::NONE),
	stringDecimals(3),
	showExtendedEditor(false),
	extendedEditorInvertX(false),
	extendedEditorInvertY(false),
	extendedEditorStretchMode(false),
	canShowExtendedEditor(true)
{
	canHaveRange = true;

	value = var();
	value.append(0.f);
	value.append(0.f);

	defaultValue = var();
	defaultValue.append(0.f);
	defaultValue.append(0.f);

	minimumValue = var();
	minimumValue.append((float)INT32_MIN);
	minimumValue.append((float)INT32_MIN);

	maximumValue = var();
	maximumValue.append((float)INT32_MAX);
	maximumValue.append((float)INT32_MAX);

	//hideInEditor = true;
	argumentsDescription = "float, float";
}


void Point2DParameter::setPoint(Point<float> point)
{
	setPoint(point.x, point.y);
}

void Point2DParameter::setPoint(float _x, float _y)
{
	var d;
	d.append(_x);
	d.append(_y);
	setValue(d);
}

UndoableAction* Point2DParameter::setUndoablePoint(Point<float> oldPoint, Point<float> newPoint, bool onlyReturnAction)
{
	return setUndoablePoint(oldPoint.x, oldPoint.y, newPoint.x, newPoint.y, onlyReturnAction);
}

UndoableAction* Point2DParameter::setUndoablePoint(float oldX, float oldY, float newX, float newY, bool onlyReturnAction)
{
	var od;
	od.append(oldX);
	od.append(oldY);
	var d;
	d.append(newX);
	d.append(newY);

	if (checkValueIsTheSame(od, d) && !alwaysNotify) return nullptr;

	return setUndoableValue(od, d, onlyReturnAction);
}

void Point2DParameter::setValueInternal(var& _value)
{
	if (_value.size() < 2)
	{
		jassertfalse;
		return;
	}

	_value[0] = (float)_value[0];
	_value[1] = (float)_value[1];

	Parameter::setValueInternal(_value);

	x = _value[0];
	y = _value[1];
}

void Point2DParameter::setBounds(float _minX, float _minY, float _maxX, float _maxY)
{
	var minRange;
	var maxRange;
	minRange.append(_minX);
	minRange.append(_minY);
	maxRange.append(_maxX);
	maxRange.append(_maxY);
	setRange(minRange, maxRange);
}

bool Point2DParameter::setAttributeInternal(String name, var val)
{
	if (name == "ui")
	{
		if (val == "time") defaultUI = FloatParameter::TIME;
		else if (val == "slider") defaultUI = FloatParameter::SLIDER;
		else if (val == "stepper") defaultUI = FloatParameter::STEPPER;
		else if (val == "label") defaultUI = FloatParameter::LABEL;
	}
	else if (name == "canvasInvertX") extendedEditorInvertX = (bool)val;
	else if (name == "canvasInvertY") extendedEditorInvertY = (bool)val;
	else if (name == "canvasStretchMode") extendedEditorStretchMode = (bool)val;
	else if(name == "stringDecimals") stringDecimals = (int)val;
	else
	{
		return Parameter::setAttributeInternal(name, val);
	}

	return true;
}

StringArray Point2DParameter::getValidAttributes() const
{
	StringArray att = Parameter::getValidAttributes();
	att.addArray({ "ui", "canvasInvertX","canvasInvertY","canvasStretchMode", "stringDecimals"});
	return att;
}

StringArray Point2DParameter::getValuesNames()
{
	return StringArray("X", "Y");
}

Point<float> Point2DParameter::getPoint() {
	return Point<float>(x, y);
}

var Point2DParameter::getLerpValueTo(var targetValue, float weight)
{
	if (!targetValue.isArray()) return value;
	var result;
	result.append(jmap(weight, x, (float)targetValue[0]));
	result.append(jmap(weight, y, (float)targetValue[1]));
	return result;
}

void Point2DParameter::setWeightedValue(Array<var> values, Array<float> weights)
{
	jassert(values.size() == weights.size());

	float tValues[2];
	tValues[0] = tValues[1] = 0;

	for (int i = 0; i < values.size(); ++i)
	{
		jassert(values[i].size() == 2);
		for (int j = 0; j < 2; j++)
		{
			tValues[j] += (float)values[i][j] * weights[i];
		}
	}

	setPoint(tValues[0], tValues[1]);
}

void Point2DParameter::setShowExtendedEditor(bool val)
{
	if (val == showExtendedEditor) return;
	showExtendedEditor = val;
	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::UI_PARAMS_CHANGED, this));
}

bool Point2DParameter::checkValueIsTheSame(var newValue, var oldValue)
{
	if (!(newValue.isArray() && oldValue.isArray())) return false;
	if (newValue.size() == 0 || oldValue.size() == 0) return false;

	bool result = newValue[0] == oldValue[0] && newValue[1] == oldValue[1];

	return result;
}

ControllableUI* Point2DParameter::createDefaultUI(Array<Controllable*> controllables)
{
	Array<Point2DParameter*> parameters = getArrayAs<Controllable, Point2DParameter>(controllables);
	if (parameters.size() == 0) parameters.add(this);
	return new DoubleSliderUI(parameters);
}

var Point2DParameter::getJSONDataInternal()
{
	var data = Parameter::getJSONDataInternal();
	if (showExtendedEditor)
	{
		data.getDynamicObject()->setProperty("extendedEditor", showExtendedEditor);
		if (extendedEditorInvertX) data.getDynamicObject()->setProperty("extendedEditorInvertX", extendedEditorInvertX);
		if (extendedEditorInvertY) data.getDynamicObject()->setProperty("extendedEditorInvertY", extendedEditorInvertY);
		if (extendedEditorStretchMode) data.getDynamicObject()->setProperty("extendedEditorStretchMode", extendedEditorStretchMode);
	}
	return data;
}

void Point2DParameter::loadJSONDataInternal(var data)
{
	Parameter::loadJSONDataInternal(data);
	showExtendedEditor = data.getProperty("extendedEditor", showExtendedEditor);
	extendedEditorInvertX = data.getProperty("extendedEditorInvertX", extendedEditorInvertX);
	extendedEditorInvertY = data.getProperty("extendedEditorInvertY", extendedEditorInvertY);
	extendedEditorStretchMode = data.getProperty("extendedEditorStretchMode", extendedEditorStretchMode);
}

var Point2DParameter::getCroppedValue(var originalValue)
{
	jassert(originalValue.isArray() && minimumValue.isArray() && maximumValue.isArray());
	if (!originalValue.isArray() || originalValue.size() < 2)
	{
		var val;
		val.append((int)originalValue);
		val.append(0);
		return val;
	}

	if (minimumValue.size() < 2 || maximumValue.size() < 2) return originalValue;

	var val;
	for (int i = 0; i < 2; ++i) val.append(jlimit(minimumValue[i], maximumValue[i], originalValue[i]));
	return val;
}
