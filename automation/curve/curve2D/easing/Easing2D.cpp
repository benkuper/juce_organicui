/*
  ==============================================================================

	Easing2D.cpp
	Created: 21 Mar 2020 4:06:44pm
	Author:  bkupe

  ==============================================================================
*/

#include "ui/Easing2DUI.h"

const String Easing2D::typeNames[Easing2D::TYPE_MAX] = { "Linear", "Bezier" };

Easing2D::Easing2D(Type type) :
	ControllableContainer("Easing"),
	type(type),
	length(0)
{
	showInspectorOnSelect = false;
}

Easing2D::~Easing2D()
{
	masterReference.clear();
}

void Easing2D::updateKeys(const Point<float>& _start, const Point<float>& _end, bool updateKeys)
{
	start = _start;
	end = _end;
	if(updateKeys) updateLength();
}


LinearEasing2D::LinearEasing2D() :
	Easing2D(LINEAR)
{
}

Point<float> LinearEasing2D::getValue(const float& weight)
{
	return start + (end - start) * weight;
}

void LinearEasing2D::updateLength()
{
	length = end.getDistanceFrom(start);
}

Rectangle<float> LinearEasing2D::getBounds(bool)
{
	return Rectangle<float>(Point<float>(jmin(start.x, end.x), jmin(start.y, end.y)), Point<float>(jmax(start.x, end.x), jmax(start.y, end.y)));
}

Point<float> LinearEasing2D::getClosestPointForPos(Point<float> pos)
{
	if (start == end) return start;
	
	float dx = end.x-start.x;
	float dy = end.y - start.y;

	// Calculate the t that minimizes the distance.
	float t = ((pos.x - start.x) * dx + (pos.y - start.y) * dy) / (dx * dx + dy * dy);

	// See if this represents one of the segment's
	// end points or a point in the middle.
	if (t <= 0) return start;
	else if (t >= 1) return end;
	else return Point<float>(start.x + t * dx, start.y + t * dy);
}

Easing2DUI* LinearEasing2D::createUI()
{
	return new LinearEasing2DUI(this);
}

CubicEasing2D::CubicEasing2D() :
	Easing2D(BEZIER)
{
	anchor1 = addPoint2DParameter("Anchor 1", "First anchor for this interpolation");
	anchor2 = addPoint2DParameter("Anchor 2", "Second anchor for this interpolation");
}

void CubicEasing2D::updateKeys(const Point<float>& _start, const Point<float>& _end, bool updateKeys)
{
	Easing2D::updateKeys(_start, _end, false);
	/*if (!anchor1->isOverriden)
	{
		Point<float> v = (_end - _start) * .3f;
		var dv;
		dv.append(v.x);
		dv.append(v.y);
		anchor1->setValue(dv);
		//anchor1->resetValue();
	}

	if (!anchor2->isOverriden)
	{
		Point<float> v = (_start - _end) * .3f;
		var dv;
		dv.append(v.x);
		dv.append(v.y);
		anchor2->setValue(dv);
		//anchor2->resetValue();
	}
	*/
	if(updateKeys) updateBezier();
}

void CubicEasing2D::updateBezier()
{
	Point<float> a1 = start + anchor1->getPoint();
	Point<float> a2 = end + anchor2->getPoint();

	bezier = Bezier::Bezier<3>({ {start.x, start.y},{a1.x, a1.y},{a2.x,a2.y},{end.x,end.y} });

	updateLength();
}

Point<float> CubicEasing2D::getValue(const float& weight)
{
	if (weight <= 0 || length == 0) return start;
	if (weight >= 1) return end;

	Bezier::Point p;
	p = bezier.valueAt(weight);
	return Point<float>(p.x, p.y);
}

void CubicEasing2D::updateLength()
{
	float result = 0;
	Point<float> a1 = start + anchor1->getPoint();
	Point<float> a2 = end + anchor2->getPoint();
	getBezierLength(start, a1, a2, end, 5, result);

	length = result;
}

void CubicEasing2D::getBezierLength(Point<float> A, Point<float> B, Point<float> C, Point<float> D, int precision, float& _length)
{
	if (precision > 0)
	{
		Point<float> a = A + (B - A) * 0.5f;
		Point<float> b = B + (C - B) * 0.5f;
		Point<float> c = C + (D - C) * 0.5f;
		Point<float> d = a + (b - a) * 0.5f;
		Point<float> e = b + (c - b) * 0.5f;
		Point<float> f = d + (e - d) * 0.5f;

		// left branch
		getBezierLength(A, a, d, f, precision - 1, _length);
		// right branch
		getBezierLength(f, e, c, D, precision - 1, _length);
	}
	else
	{
		float controlNetLength = (B - A).getDistanceFromOrigin() + (C - B).getDistanceFromOrigin() + (D - C).getDistanceFromOrigin();
		float chordLength = (D - A).getDistanceFromOrigin();
		_length += (chordLength + controlNetLength) / 2.0f;
	}
}

Rectangle<float> CubicEasing2D::getBounds(bool includeHandles)
{
	Bezier::AxisAlignedBoundingBox  bbox = bezier.aabb();
	Array<Point<float>> points;
	points.add(Point<float>(bbox.minX(), bbox.minY()), Point<float>(bbox.maxX(), bbox.maxY()));
	if(includeHandles) points.add(anchor1->getPoint() + start, anchor2->getPoint() + end);

	return Rectangle<float>::findAreaContainingPoints(points.getRawDataPointer(), points.size());
}

Point<float> CubicEasing2D::getClosestPointForPos(Point<float> pos)
{
	const int precision = length*30;
	Point<float> closestP;
	float minDist = INT32_MAX;
	for (int i = 0; i < precision; i++)
	{
		Bezier::Point bp = bezier.valueAt(i * 1.0f / precision);
		Point<float> p(bp.x,bp.y);
		float dist = p.getDistanceSquaredFrom(pos);
		if (dist < minDist)
		{
			closestP.setXY(p.x, p.y);
			minDist = dist;
		}
	}

	return closestP;
}

void CubicEasing2D::onContainerParameterChanged(Parameter* p)
{
	updateBezier();
}

Easing2DUI* CubicEasing2D::createUI()
{
	return new CubicEasing2DUI(this);
}