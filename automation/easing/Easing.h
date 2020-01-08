/*
  ==============================================================================

    Easing.h
    Created: 11 Dec 2016 1:29:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class EasingUI;

class EasingBase :
	public ControllableContainer
{
public:
	enum Type { LINEAR, BEZIER, HOLD, SINE };

	EasingBase(Type type);
	virtual ~EasingBase();
	
	Type type;

	virtual EasingUI* createUI() {
		jassertfalse;
		return nullptr;
	}; //must be overriden

private :
	WeakReference<EasingBase>::Master masterReference;
	friend class WeakReference<EasingBase>;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasingBase)
};

template<class T>
class Easing :
	public EasingBase
{
public:
	Easing(Type type) : EasingBase(type) {}
	virtual ~Easing() {}
	
	virtual T getValue(const T& start, const T& end, const float& weight) = 0;//must be overriden
};


template<class T>
class LinearEasing :
	public Easing<T>
{
public:
	LinearEasing();

	T getValue(const T &start, const T &end, const float &weight) override;

	EasingUI * createUI() override;
};

template<class T>
class HoldEasing :
	public Easing<T>
{
public:
	HoldEasing();

	virtual T getValue(const T &start, const T &end, const float &weight) override;

	EasingUI * createUI() override;
};

/*
template<class T>
class CubicEasing :
	public Easing<T>
{
public:
	CubicEasing();
	Point2DParameter * anchor1;
	Point2DParameter * anchor2;

	virtual float getValue(const T &start, const T &end, const float &weight) override;

	void onContainerParameterChanged(Parameter * p) override;
	class Bezier1D
	{
	public:
		Point<float> a;
		Point<float> b;
		Point<float> c;

		const float epsilon = 1e-6f; //Precision

		void setup(const Point<float> &a1, const Point<float> &a2);

		float sampleCurveX(float t);
		float sampleCurveY(float t);
		float sampleCurveDerivativeX(float t);

		float getValueForX(const float &tx);

		float solveCurveX(const float &tx);
	};

	Bezier bezier;
	//Array<float> solveCubic(float a, float b, float c, float d);

	EasingUI * createUI() override;
};

template<class T>
class SineEasing :
	public Easing<T>
{
public:
	SineEasing();
	Point2DParameter * freqAmp;

	virtual T getValue(const T &start, const T &end, const float &weight) override;

	EasingUI * createUI() override;
};
*/

template<class T> LinearEasing<T>::LinearEasing() : EasingBase(LINEAR) {}
template<class T> HoldEasing<T>::HoldEasing() : EasingBase(HOLD) {}

template<class T>
EasingUI* LinearEasing<T>::createUI()
{
	return new LinearEasingUI(this);
}

template<class T>
EasingUI* HoldEasing<T>::createUI()
{
	return new HoldEasingUI(this);
}


template<class T>
T LinearEasing<T>::getValue(const T& start, const T& end, const float& weight)
{
	return start + (end - start) * weight;
}


template<class T>
T HoldEasing<T>::getValue(const T& start, const T& end, const float& weight)
{
	if (weight >= 1) return end;
	return start;
}

/*
template<class T>
CubicEasing::CubicEasing() :
	Easing(BEZIER)
{
	anchor1 = addPoint2DParameter("Anchor 1", "Anchor 1 of the quadratic curve");
	anchor2 = addPoint2DParameter("Anchor 2", "Anchor 2 of the quadratic curve");

	anchor1->setBounds(0, -1, .99f, 2);
	anchor2->setBounds(.01f, -1, 1, 2);

	anchor1->setPoint(.5f, 0);
	anchor2->setPoint(.5f, 1);

	bezier.setup(anchor1->getPoint(), anchor2->getPoint());
}

template<class T>
void CubicEasing::onContainerParameterChanged(Parameter* p)
{
	if (p == anchor1 || p == anchor2) bezier.setup(anchor1->getPoint(), anchor2->getPoint());
}

template<class T>
float CubicEasing::getValue(const float& start, const float& end, const float& weight)
{
	if (weight <= 0) return start;
	if (weight >= 1) return end;

	float val = bezier.getValueForX(weight);
	return jmap<float>(val, start, end);
}

template<class T>
void CubicEasing<T>::Bezier1D::setup(const Point<float>& a1, const Point<float>& a2)
{
	c.setXY(3 * a1.x, 3 * a1.y);
	b.setXY(3 * (a2.x - a1.x) - c.x, 3 * (a2.y - a1.y) - c.y);
	a.setXY(1 - c.x - b.x, 1 - c.y - b.y);
}

template<class T>
float CubicEasing<T>::Bezier::sampleCurveX(float t) {
	return ((a.x * t + b.x) * t + c.x) * t;
}

template<class T>
float CubicEasing<T>::Bezier::sampleCurveY(float t) {
	return ((a.y * t + b.y) * t + c.y) * t;
}

template<class T>
float CubicEasing<T>::Bezier::sampleCurveDerivativeX(float t) {
	return (3 * a.x * t + 2 * b.x) * t + c.x;
}

template<class T>
float CubicEasing<T>::Bezier::getValueForX(const float& tx)
{
	return sampleCurveY(solveCurveX(tx));
}

template<class T>
float CubicEasing<T>::Bezier::solveCurveX(const float& tx)
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

template<class T>
EasingUI* CubicEasing<T>::createUI()
{
	return new CubicEasingUI(this);
}



template<class T>
SineEasing::SineEasing() :
	Easing(SINE)
{
	freqAmp = addPoint2DParameter("Anchor 1", "Anchor 1 of the quadratic curve");
	freqAmp->setBounds(.01f, -1, 1, 2);
	freqAmp->setPoint(.2f, .5f);
}

template<class T>
float SineEasing::getValue(const float& start, const float& end, const float& weight)
{
	//DBG(freqAmp->getPoint().toString() << " / " << sinf(weight / freqAmp->x));
	return start + (end - start) * weight + sinf(weight * float_Pi * 2 / freqAmp->x) * freqAmp->y;
}

template<class T>
EasingUI* SineEasing::createUI()
{
	return new SineEasingUI(this);
}
*/