#include "Easing.h"
/*
  ==============================================================================

	Easing.cpp
	Created: 11 Dec 2016 1:29:02pm
	Author:  Ben

  ==============================================================================
*/

const String Easing::typeNames[Easing::TYPE_MAX]{"Linear", "Bezier", "Hold","Sine"};
 
Easing::Easing(Type type) :
	ControllableContainer("Easing"),
	type(type),
	length(0)
{
	showInspectorOnSelect = false;
}

Easing::~Easing()
{
	masterReference.clear();
}

void Easing::updateKeys(const Point<float>& _start, const Point<float>& _end)
{
	if (start == _start && end == _end) return;
	start = _start;
	end = _end;
	length = end.x - start.x;

	updateKeysInternal();
}

EasingUI* Easing::createUI()
{
	return new EasingUI(this);
}


LinearEasing::LinearEasing() : Easing(LINEAR) {}
HoldEasing::HoldEasing() : Easing(HOLD) {}


EasingUI * LinearEasing::createUI()
{
	return new LinearEasingUI(this);
}


EasingUI* HoldEasing::createUI()
{
	return new HoldEasingUI(this);
}

EasingUI* CubicEasing::createUI()
{
	return new CubicEasingUI(this);
}


float LinearEasing::getValue(const float & weight)
{
	return jmap(weight, start.y, end.y);
}

Rectangle<float> LinearEasing::getBounds(bool includeHandles)
{
	return 	Rectangle<float>(Point<float>(jmin(start.x, end.x), jmin(start.y, end.y)), Point<float>(jmax(start.x, end.x), jmax(start.y, end.y)));
}


float HoldEasing::getValue(const float & weight)
{
	if (weight < 1) return  start.y;
	return end.y;
}

Rectangle<float> HoldEasing::getBounds(bool includeHandles)
{
	return 	Rectangle<float>(Point<float>(jmin(start.x, end.x), jmin(start.y, end.y)), Point<float>(jmax(start.x, end.x), jmax(start.y, end.y)));
}


CubicEasing::CubicEasing() :
	Easing(BEZIER)
{
	anchor1 = addPoint2DParameter("Anchor 1", "Anchor 1 of the quadratic curve");
	anchor2 = addPoint2DParameter("Anchor 2", "Anchor 2 of the quadratic curve");


}


void CubicEasing::onContainerParameterChanged(Parameter* p)
{
	if (p == anchor1 || p == anchor2) updateBezier();
}

Rectangle<float> CubicEasing::getBounds(bool includeHandles)
{
	Bezier::AxisAlignedBoundingBox  bbox = bezier.aabb();
	Array<Point<float>> points;
	points.add(Point<float>(bbox.minX(), bbox.minY()), Point<float>(bbox.maxX(), bbox.maxY()));
	if (includeHandles) points.add(anchor1->getPoint() + start, anchor2->getPoint() + end);

	return Rectangle<float>::findAreaContainingPoints(points.getRawDataPointer(), points.size());
}


float CubicEasing::getValue(const float & weight)
{
	if (length == 0 || weight <= 0 || uniformLUT.size() == 0) return start.y;
	if (weight >= 1) return end.y;

	float indexF = weight * (uniformLUT.size() - 1);
	int index = (int)floor(indexF);
	float rel = indexF - index;
	float p1 = uniformLUT[index];
	float p2 = uniformLUT[index + 1];

	float p = p1 + (p2 - p1) * rel;
	return p;
}

Point<float> CubicEasing::getRawValue(const float& weight)
{
	Bezier::Point p = bezier.valueAt(weight);
	return { p.x, p.y };
}

void CubicEasing::updateKeysInternal()
{
	if (length == 0) return;
	anchor1->setBounds(0, INT32_MIN, length, INT32_MAX);
	anchor2->setBounds(-length, INT32_MIN, 0, INT32_MAX);

	if (!anchor1->isOverriden && !anchor2->isOverriden)
	{
		anchor1->setPoint(length * .3f, 0);
		anchor2->setPoint(-length * .3f, 0);
	}

	updateBezier();
}

void CubicEasing::updateBezier()
{
	if (length == 0) return;
	
	Point<float> a1 = start + anchor1->getPoint();
	Point<float> a2 = end + anchor2->getPoint();
	bezier = Bezier::Bezier<3>({ {start.x, start.y},{a1.x, a1.y},{a2.x,a2.y},{end.x,end.y} });

	updateUniformLUT(1 + length * 50);
}

void CubicEasing::updateUniformLUT(int precision)
{
	uniformLUT.clear();
	Array<float> arcLengths;
	arcLengths.add(0);

	Point<float> prevP = getRawValue(0);

	float dist = 0;
	for (int i = 1; i <= precision; i++) {
		float rel = i * 1.0f / precision;
		Point<float> p = getRawValue(rel);

		dist += p.x - prevP.x;
		arcLengths.add(dist);
		prevP.setXY(p.x, p.y);
	}

	for (int i = 0; i <= precision; i++)
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
		uniformLUT.add(curPoint.y);
	}
}


SineEasing::SineEasing() :
	Easing(SINE)
{
	freqAmp = addPoint2DParameter("Frequency Amplitude", "Frequency and amplitude of the sine wave");
	freqAmp->setBounds(.01f, -1, 1, 2);
	freqAmp->setPoint(1.f, .25f);
}

void SineEasing::updateKeysInternal()
{
	if (length == 0) return;
	freqAmp->setBounds(0, INT32_MIN, length, INT32_MAX);
}

float SineEasing::getValue(const float & weight)
{
	return  start.y + (end.y - start.y) * weight + sinf(weight * length * float_Pi * 2 / freqAmp->x) * freqAmp->y;
}

Rectangle<float> SineEasing::getBounds(bool includeHandles)
{
	Array<Point<float>> points;
	points.add(Point<float>(start.x, start.y - std::abs(freqAmp->y)), Point<float>(end.x, end.y + std::abs(freqAmp->y)));
	return Rectangle<float>::findAreaContainingPoints(points.getRawDataPointer(), points.size());
}


EasingUI* SineEasing::createUI()
{
	return new SineEasingUI(this);
}

