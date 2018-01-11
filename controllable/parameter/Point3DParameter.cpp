/*
==============================================================================

Point3DParameter.cpp
Created: 30 Sep 2016 9:37:36am
Author:  bkupe

==============================================================================
*/

Point3DParameter::Point3DParameter(const String & niceName, const String & description, bool enabled) :
	Parameter(POINT3D, niceName, description, 0, 0, 1, enabled),
    x(0), y(0),z(0)
{

	value = var();
	value.append(0);
	value.append(0);
	value.append(0);


	minimumValue = var();
	minimumValue.append(0);
	minimumValue.append(0);
	minimumValue.append(0);

	maximumValue = var();
	maximumValue.append(1);
	maximumValue.append(1);
	maximumValue.append(1);

	//hideInEditor = true;
	argumentsDescription = "float, float, float";
}

void Point3DParameter::setVector(Vector3D<float> _value)
{
	setVector(_value.x, _value.y,_value.z);
}

void Point3DParameter::setVector(float _x, float _y, float _z)
{
	var d;
	if (autoAdaptRange)
	{
		d.append(_x);
		d.append(_y);
		d.append(_z);
	} else
	{
		d.append(jlimit<float>(minimumValue[0], maximumValue[0], _x));
		d.append(jlimit<float>(minimumValue[1], maximumValue[1], _y));
		d.append(jlimit<float>(minimumValue[2], maximumValue[2], _z));
	}
	
	if (d[0] == value[0] && d[1] == value[1] && d[2] == value[2]) return;
	setValue(d);
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

	x = jlimit<float>(minimumValue[0], maximumValue[0], _value[0]);
	y = jlimit<float>(minimumValue[1], maximumValue[1], _value[1]);
	z = jlimit<float>(minimumValue[2], maximumValue[2], _value[2]);

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

Vector3D<float> Point3DParameter::getVector() {
	return Vector3D<float>(x, y,z);
}

bool Point3DParameter::checkValueIsTheSame(var newValue, var oldValue)
{
	if ( !(newValue.isArray() && oldValue.isArray())) return false;

	return newValue[0] == oldValue[0] && newValue[1] == oldValue[1] && newValue[2] == oldValue[2];
}

ControllableUI * Point3DParameter::createDefaultUI(Controllable * targetControllable)
{
	return new TripleSliderUI(targetControllable != nullptr ? (Point3DParameter *)targetControllable : this);
}
