#include "Easing.h"
/*
  ==============================================================================

    Easing.cpp
    Created: 11 Dec 2016 1:29:02pm
    Author:  Ben

  ==============================================================================
*/

Easing::Easing(Type _type, AutomationKey * k1, AutomationKey * k2, int dimension) :
	ControllableContainer("ease"),
	k1(k1),
	k2(k2),
	length(1),
	dimension(dimension),
	type(_type)
{
	//showInspectorOnSelect = false;

	updateFromKeys();
	helpID = "Easing";
}

Easing::~Easing()
{
	masterReference.clear();
}

void Easing::setNextKey(AutomationKey* k)
{
	if (k2 == k) return;
	k2 = k;
	updateFromKeys();
}

void Easing::updateFromKeys()
{
	if (k2 != nullptr)
	{
		length = k2->position->floatValue() - k1->position->floatValue();
		updateFromKeysInternal();
	}
}

//must be overriden

inline const float Easing::getKeyValue(AutomationKey* k) { 
	return k != nullptr ? k->getDimensionValue(dimension) : 0; 
}


LinearEasing::LinearEasing(AutomationKey* k1, AutomationKey* k2, int dimension) : Easing(LINEAR, k1, k2, dimension) {}
HoldEasing::HoldEasing(AutomationKey* k1, AutomationKey* k2, int dimension) : Easing(HOLD, k1, k2, dimension) {}


EasingUI * LinearEasing::createUI()
{
	return new LinearEasingUI(this);
}


EasingUI * HoldEasing::createUI()
{
	return new HoldEasingUI(this);
}

EasingUI * CubicEasing::createUI()
{
	return new CubicEasingUI(this);
}


float LinearEasing::getValue(const float & weight)
{
	return jmap(weight, getKeyValue(k1), getKeyValue(k2));
}


float HoldEasing::getValue(const float & weight)
{
	if (weight >= 1) return  getKeyValue(k1);
	return  getKeyValue(k2);
}


CubicEasing::CubicEasing(AutomationKey* k1, AutomationKey* k2, int dimension) :
	Easing(BEZIER, k1, k2, dimension)
{
	k1Anchor = addPoint2DParameter("Anchor 1", "Anchor 1 of the quadratic curve");
	k2Anchor = addPoint2DParameter("Anchor 2", "Anchor 2 of the quadratic curve");

	k1Anchor->setPoint(length * .3f, 0);
	k2Anchor->setPoint(length * .3f, 0);
	
	updateBezier();
}


void CubicEasing::onContainerParameterChanged(Parameter* p)
{
	if (p == k1Anchor || p == k2Anchor) updateBezier();
}


float CubicEasing::getValue(const float & weight)
{
	if (weight <= 0 || k2 == nullptr) return getKeyValue(k1);
	if (weight >= 1) return getKeyValue(k2);

	int numLUT = bezierLUT.size();
	if (numLUT == 0) return getKeyValue(k1);

	float tx = weight * length; //use this to figure out
	
	Point<float> prevPoint = bezierLUT[0];
	for (int i = 1; i < numLUT; i++)
	{
		Point<float> p = bezierLUT[i];
		if (p.x >= tx)
		{
			return jmap(tx, prevPoint.x, p.x, prevPoint.y, p.y);
		}
		prevPoint.setXY(p.x, p.y);
	}
	
	return getKeyValue(k1);
}

void CubicEasing::updateFromKeysInternal()
{
	k1Anchor->setBounds(0, INT32_MIN, length, INT32_MAX);
	k2Anchor->setBounds(0, INT32_MIN, length, INT32_MAX);
	updateBezier();
}

void CubicEasing::updateBezier()
{
	if (k1 == nullptr || k2 == nullptr) return;

	float v1 = getKeyValue(k1);
	float v2 = getKeyValue(k2);

	Bezier::Point a(0, v1);
	Bezier::Point b(k1Anchor->x, v1+k1Anchor->y);
	Bezier::Point c(length - k2Anchor->x, v2+k2Anchor->y);
	Bezier::Point d(length, v2);
	bezier = Bezier::Bezier<3>({ a, b, c, d });

	//Generate LUT
	const int lutPrecision = 50*length;
	bezierLUT.clear();
	for (int i = 0; i < lutPrecision; i++) {
		Bezier::Point p = bezier.valueAt(i * length / lutPrecision);
		bezierLUT.add({ p.x,p.y });
	}
}

/*
void CubicEasing::Bezier::setup(const Point<float>& a1, const Point<float>& a2)
{
	c.setXY(3 * a1.x, 3 * a1.y);
	b.setXY(3 * (a2.x - a1.x) - c.x, 3 * (a2.y - a1.y) - c.y);
	a.setXY(1 - c.x - b.x, 1 - c.y - b.y);
}

inline float CubicEasing::Bezier::sampleCurveX(float t) {
	return ((a.x * t + b.x) * t + c.x) * t;
}

inline float CubicEasing::Bezier::sampleCurveY(float t) {
	return ((a.y * t + b.y) * t + c.y) * t;
}

inline float CubicEasing::Bezier::sampleCurveDerivativeX(float t) {
	return (3 * a.x * t + 2 * b.x) * t + c.x;
}

float CubicEasing::Bezier::getValueForX(const float & tx)
{
	return sampleCurveY(solveCurveX(tx));
}

float CubicEasing::Bezier::solveCurveX(const float & tx)
{
	float t0;
	float t1;
	float t2;
	float x2;
	float d2;
	float i;

	// First try a few iterations of Newton's method -- normally very fast.
	for (t2 = tx, i = 0; i < 8; i++) {
		x2 = sampleCurveX(t2) - tx;
		if (std::abs(x2) < epsilon)
			return t2;
		d2 = sampleCurveDerivativeX(t2);
		if (std::abs(d2) < epsilon)
			break;
		t2 = t2 - x2 / d2;
	}

	// No solution found - use bi-section
	t0 = 0.0;
	t1 = 1.0;
	t2 = tx;

	if (t2 < t0) return t0;
	if (t2 > t1) return t1;

	while (t0 < t1) {
		x2 = sampleCurveX(t2);
        if (std::abs(x2 - tx) < epsilon)
			return t2;
		if (tx > x2) t0 = t2;
		else t1 = t2;

		t2 = (t1 - t0) * .5f + t0;
	}

	// Give up
	return t2;
}
*/

SineEasing::SineEasing(AutomationKey* k1, AutomationKey* k2, int dimension) :
	Easing(SINE, k1, k2, dimension)
{
	freqAmp = addPoint2DParameter("Anchor 1", "Anchor 1 of the quadratic curve");
	freqAmp->setBounds(.01f, -1, 1, 2);
	freqAmp->setPoint(.2f, .5f);
}

float SineEasing::getValue(const float & weight)
{
	//DBG(freqAmp->getPoint().toString() << " / " << sinf(weight / freqAmp->x));
	return 0;// start + (end - start) * weight + sinf(weight * float_Pi * 2 / freqAmp->x) * freqAmp->y;
}


EasingUI * SineEasing::createUI()
{
	return new SineEasingUI(this);
}
