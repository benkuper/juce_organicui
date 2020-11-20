#include "Point3DParameter.h"
/*
==============================================================================

Point3DParameter.cpp
Created: 30 Sep 2016 9:37:36am
Author:  bkupe

==============================================================================
*/

Point3DParameter::Point3DParameter(const String & niceName, const String & description, bool enabled) :
	Parameter(POINT3D, niceName, description, 0, 0, 1, enabled),
	x(0), y(0), z(0),
	defaultUI(FloatParameter::NONE)
{
	canHaveRange = true;

	value = var();
	value.append(0);
	value.append(0);
	value.append(0);

	defaultValue = var();
	defaultValue.append(0);
	defaultValue.append(0);
	defaultValue.append(0);
	 
	minimumValue = var();
	minimumValue.append(INT32_MIN);
	minimumValue.append(INT32_MIN);
	minimumValue.append(INT32_MIN);

	maximumValue = var();
	maximumValue.append(INT32_MAX);
	maximumValue.append(INT32_MAX);
	maximumValue.append(INT32_MAX);

	//hideInEditor = true;
	argumentsDescription = "float, float, float";
}

void Point3DParameter::setVector(Vector3D<float> _value)
{
	setVector(_value.x, _value.y, _value.z);
}

void Point3DParameter::setVector(float _x, float _y, float _z)
{
	var d;
	d.append(_x);
	d.append(_y);
	d.append(_z);
	setValue(d);
}

void Point3DParameter::setUndoableVector(Vector3D<float> oldVector, Vector3D<float> newVector)
{
	setUndoableVector(oldVector.x, oldVector.y, oldVector.z, newVector.x, newVector.y, newVector.z);
}

void Point3DParameter::setUndoableVector(float oldX, float oldY, float oldZ, float newX, float newY, float newZ)
{
	var od;
	od.append(oldX);
	od.append(oldY);
	od.append(oldZ);
	var d;
	d.append(newX);
	d.append(newY);
	d.append(newZ);

	if (checkValueIsTheSame(od, d) && !alwaysNotify) return;

	setUndoableValue(od, d);
}

void Point3DParameter::setValueInternal(var& _value)
{
	Parameter::setValueInternal(_value);
	if (value.size() < 3)
	{
		jassertfalse;
		return;
	}

	x = _value[0];
	y = _value[1];
	z = _value[2];
}

void Point3DParameter::setBounds(float _minX, float _minY, float _minZ, float _maxX, float _maxY, float _maxZ)
{
	var minRange;
	var maxRange;
	minRange.append(_minX);
	minRange.append(_minY);
	minRange.append(_minZ);
	maxRange.append(_maxX);
	maxRange.append(_maxY);
	maxRange.append(_maxZ);
	setRange(minRange, maxRange);
}

Vector3D<float> Point3DParameter::getVector() {
	return Vector3D<float>(x, y, z);
}

var Point3DParameter::getLerpValueTo(var targetValue, float weight)
{
	if (!targetValue.isArray()) return value;
	var result;
	result.append(jmap(weight, x, (float)targetValue[0]));
	result.append(jmap(weight, y, (float)targetValue[1]));
	result.append(jmap(weight, z, (float)targetValue[2]));
	return result;
}

void Point3DParameter::setWeightedValue(Array<var> values, Array<float> weights)
{
	jassert(values.size() == weights.size());

	float tValues[3];
	tValues[0] = tValues[1] = tValues[2] = 0;

	for (int i = 0; i < values.size(); ++i)
	{
		jassert(values[i].size() == 3);
		for (int j = 0; j < 3; j++)
		{
			tValues[j] += (float)values[i][j] * weights[i];
		}
	}

	setVector(tValues[0], tValues[1], tValues[2]);
}


bool Point3DParameter::checkValueIsTheSame(var newValue, var oldValue)
{
	if (!(newValue.isArray() && oldValue.isArray())) return false;
	if (newValue.size() < 3 || oldValue.size() < 3)
	{
		DBG("Problem with range !");
		return false;
	}
	bool result = newValue[0] == oldValue[0] && newValue[1] == oldValue[1] && newValue[2] == oldValue[2];

	return result;
}

StringArray Point3DParameter::getValuesNames()
{
	return StringArray("X", "Y", "Z");
}

ControllableUI* Point3DParameter::createDefaultUI()
{
	return new TripleSliderUI(this);
}

var Point3DParameter::getCroppedValue(var originalValue)
{
	if (originalValue.size() < 3 || minimumValue.size() < 3 || maximumValue.size() < 3)
	{
		LOGWARNING("Value is of the wrong format ! ");
		return originalValue;
	}

	var val;
	for (int i = 0; i < 3; ++i) val.append(jlimit(minimumValue[i], maximumValue[i], originalValue[i]));
	return val;
}
