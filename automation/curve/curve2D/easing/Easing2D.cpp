/*
  ==============================================================================

	Easing2D.cpp
	Created: 21 Mar 2020 4:06:44pm
	Author:  bkupe

  ==============================================================================
*/

#include "ui/Easing2DUI.h"
#include "Easing2D.h"

Easing2D::Easing2D(int type) :
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

void Easing2D::updateKeys(const Point<float>& _start, const Point<float>& _end, bool _updateLength)
{
	start = _start;
	end = _end;
	if(_updateLength) updateLength();
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

juce::Rectangle<float> LinearEasing2D::getBounds(bool)
{
	return juce::Rectangle<float>(Point<float>(jmin(start.x, end.x), jmin(start.y, end.y)), Point<float>(jmax(start.x, end.x), jmax(start.y, end.y)));
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
	if(updateKeys) updateBezier();
}

void CubicEasing2D::updateBezier()
{
	Point<float> a1 = start + anchor1->getPoint();
	Point<float> a2 = end + anchor2->getPoint();

	bezier = Bezier::Bezier<3>({ {start.x, start.y},{a1.x, a1.y},{a2.x,a2.y},{end.x,end.y} });

	updateLength();

	if (length == 0) return;
	updateUniformLUT(10 + length * 40);
}

void CubicEasing2D::updateUniformLUT(int precision)
{
	uniformLUT.clear();

	Array<float> arcLengths;
	arcLengths.add(0);

	Point<float> prevP = getRawValue(0);

	float dist = 0;
	for (int i = 1; i <= precision; i ++) {
		float rel =  i * 1.0f/ precision;
		Point<float> p = getRawValue(rel);

		dist += p.getDistanceFrom(prevP);
		arcLengths.add(dist);
		prevP.setXY(p.x, p.y);
	}

	for (int i = 0; i <= precision; ++i)
	{
		float targetLength = (i * 1.0f / precision) * length;
		int low = 0;
		int high = precision;
		int index = 0;

		while (low < high) {
			index = low + (((high - low) / 2) | 0);
			if (arcLengths[index] < targetLength) low = index + 1;
			else  high = index;
		}

		if (arcLengths[index] > targetLength) index--;

		float lengthBefore = arcLengths[index];

		float pos = 0;
		if (lengthBefore == targetLength) pos = index / precision;
		else pos = (index + (targetLength - lengthBefore) / (arcLengths[index + 1] - lengthBefore)) / precision;

		Point<float> curPoint = getRawValue(pos);
		uniformLUT.add(curPoint);
	}

}

Point<float> CubicEasing2D::getValue(const float& weight)
{
	if (weight <= 0 || length == 0 || uniformLUT.size() == 0) return start;
	if (weight >= 1) return end;

	float indexF = weight * (uniformLUT.size() - 1);
	int index = (int)floor(indexF);
	float rel = indexF - index;
	Point<float> p1 = uniformLUT[index];
	Point<float> p2 = uniformLUT[index + 1];

	Point<float> p = p1 + (p2 - p1) * rel;
	return p;
}


Point<float> CubicEasing2D::getRawValue(const float& weight)
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

Array<Point<float>> CubicEasing2D::getSplitControlPoints(const Point<float> &pos)
{
	float t = getBezierWeight(pos);

	Point<float> p1 = start;
	Point<float> p2 = start + anchor1->getPoint();
	Point<float> p3 = end + anchor2->getPoint();
	Point<float> p4 = end;

	//Point<float> cp1start, cp1End, cp2Start, cp2End;

	Point<float> pp12 = p1 + (p2 - p1) * t;
	Point<float> pp23 = p2 + (p3 - p2) * t;
	Point<float> pp34 = p3 + (p4 - p3) * t;

	Point<float> pp123 = pp12 + (pp23 - pp12) * t;
	Point<float> pp234 = pp23 + (pp34 - pp23) * t;
	//Point<float> pp1234 = pp123 + (pp234 - pp123) * t;

	Array<Point<float>> result;
	result.add(pp12, pp123, pp234, pp34);
	return result;
}

juce::Rectangle<float> CubicEasing2D::getBounds(bool includeHandles)
{
	Bezier::AxisAlignedBoundingBox  bbox = bezier.aabb();
	Array<Point<float>> points;
	points.add(Point<float>(bbox.minX(), bbox.minY()), Point<float>(bbox.maxX(), bbox.maxY()));
	if(includeHandles) points.add(anchor1->getPoint() + start, anchor2->getPoint() + end);

	return juce::Rectangle<float>::findAreaContainingPoints(points.getRawDataPointer(), points.size());
}

Point<float> CubicEasing2D::getClosestPointForPos(Point<float> pos)
{
	const int precision = length*30;
	Point<float> closestP;
	float minDist = INT32_MAX;
	for (int i = 0; i < precision; ++i)
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


float CubicEasing2D::getBezierWeight(const Point<float>& pos)
{
	const int precision = length * 30;
	float result = 0;
	float minDist = INT32_MAX;
	for (int i = 0; i < precision; ++i)
	{
		float weight = i * 1.0f / precision;
		Bezier::Point bp = bezier.valueAt(weight);
		Point<float> p(bp.x, bp.y);
		float dist = p.getDistanceSquaredFrom(pos);
		if (dist < minDist)
		{
			result = weight;
			minDist = dist;
		}
	}

	return result;
}


void CubicEasing2D::onContainerParameterChanged(Parameter* p)
{
	updateBezier();
}

Easing2DUI* CubicEasing2D::createUI()
{
	return new CubicEasing2DUI(this);
}
