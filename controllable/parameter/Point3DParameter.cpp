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

	if (checkValueIsTheSame(od, d)) return;

	setUndoableValue(od, d);
}

void Point3DParameter::setValueInternal(var & _value)
{
	if (!_value.isArray()) return;
	if (autoAdaptRange)
	{
		bool hasChanged = false;
		for (int i = 0; i < 3; i++)
		{
			if ((float)_value[i] < (float)minimumValue[i]) {
				minimumValue[i] = _value[i];
				hasChanged = true;
			} else if ((float)_value[i] > (float)maximumValue[i])
			{
				maximumValue[i] = _value[i];
				hasChanged = true;
			}
		}

		if (hasChanged)
		{
			setRange(minimumValue, maximumValue, false);
		}
	}

	x = std::isnan((float)_value[0]) ? 0 : jlimit<float>(minimumValue[0], maximumValue[0], _value[0]);
	y = std::isnan((float)_value[1]) ? 0 : jlimit<float>(minimumValue[1], maximumValue[1], _value[1]);
	z = std::isnan((float)_value[2]) ? 0 : jlimit<float>(minimumValue[2], maximumValue[2], _value[2]);

	value = var();
	value.append(x);
	value.append(y);
	value.append(z);
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
	setRange(minRange, maxRange, true);
}

void Point3DParameter::clearRange()
{
	setBounds(INT32_MIN, INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX, INT32_MAX);
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

	for (int i = 0; i < values.size(); i++)
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

	return newValue[0] == oldValue[0] && newValue[1] == oldValue[1] && newValue[2] == oldValue[2];
}

ControllableUI * Point3DParameter::createDefaultUI(Controllable * targetControllable)
{
	return new TripleSliderUI(targetControllable != nullptr ? (Point3DParameter *)targetControllable : this);
}
