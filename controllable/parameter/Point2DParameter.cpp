/*
  ==============================================================================

	Point2DParameter.cpp
	Created: 30 Sep 2016 9:37:36am
	Author:  bkupe

  ==============================================================================
*/

Point2DParameter::Point2DParameter(const String & niceName, const String & description, bool _enabled) :
	Parameter(POINT2D, niceName, description, 0, 0, 1, _enabled),
	x(0), y(0),
	defaultUI(FloatParameter::NONE)
{
	canHaveRange = true;

	value = var();
	value.append(0);
	value.append(0);

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

UndoableAction * Point2DParameter::setUndoablePoint(Point<float> oldPoint, Point<float> newPoint, bool onlyReturnAction)
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

void Point2DParameter::setValueInternal(var & _value)
{
	if (!_value.isArray()) return;

	x = std::isnan((float)value[0]) ? 0 : jlimit<float>(minimumValue[0], maximumValue[0], _value[0]);
	y = std::isnan((float)value[1]) ? 0 : jlimit<float>(minimumValue[1], maximumValue[1], _value[1]);

	value = var();
	value.append(x);
	value.append(y);

}

void Point2DParameter::setBounds(float _minX, float _minY, float _maxX, float _maxY)
{
	var minRange;
	var maxRange;
	minRange.append(_minX);
	minRange.append(_minY);
	maxRange.append(_maxX);
	maxRange.append(_maxY);
	setRange(minRange, maxRange, true);
}

void Point2DParameter::clearRange()
{
	setBounds(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX);
}

StringArray Point2DParameter::getValuesNames()
{
	return StringArray("X","Y");
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

	for (int i = 0; i < values.size(); i++)
	{
		jassert(values[i].size() == 2);
		for (int j = 0; j < 2; j++)
		{
			tValues[j] += (float)values[i][j] * weights[i];
		}
	}

	setPoint(tValues[0], tValues[1]);
}

bool Point2DParameter::checkValueIsTheSame(var newValue, var oldValue)
{
	if (!(newValue.isArray() && oldValue.isArray())) return false;
	if (newValue.size() == 0 || oldValue.size() == 0) return false;

	bool result = newValue[0] == oldValue[0] && newValue[1] == oldValue[1];

	return result;
}

ControllableUI * Point2DParameter::createDefaultUI(Controllable * targetControllable)
{
	return new DoubleSliderUI(targetControllable != nullptr ? (Point2DParameter *)targetControllable : this);
}
