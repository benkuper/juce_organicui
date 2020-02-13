/*
  ==============================================================================

    Easing.h
    Created: 11 Dec 2016 1:29:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Bezier.h"


class EasingUI;
class AutomationKey;

class Easing :
	public ControllableContainer
{
public:
	enum Type { LINEAR, BEZIER, HOLD, SINE };

	Easing(Type type, AutomationKey* k1, AutomationKey* k2, int dimension);
	virtual ~Easing();

	Type type;
	AutomationKey* k1;
	AutomationKey* k2;
	float length;

	int dimension;

	void setNextKey(AutomationKey* k);
	virtual void updateFromKeys();
	virtual void updateFromKeysInternal() {}

	virtual float getValue( const float &weight) = 0;//must be overriden
	const inline float getKeyValue(AutomationKey* k);

	virtual EasingUI * createUI() = 0; //must be overriden

private :
	WeakReference<Easing>::Master masterReference;
	friend class WeakReference<Easing>;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Easing)

};


class LinearEasing :
	public Easing
{
public:
	LinearEasing(AutomationKey* k1, AutomationKey* k2, int dimension);

	float getValue(const float& weight) override;

	EasingUI* createUI() override;
};

class HoldEasing :
	public Easing
{
public:
	HoldEasing(AutomationKey* k1, AutomationKey* k2, int dimension);

	virtual float getValue(const float& weight) override;
	EasingUI* createUI() override;
};

class CubicEasing :
	public Easing
{
public:
	CubicEasing(AutomationKey* k1, AutomationKey* k2, int dimension);
	Point2DParameter* k1Anchor;
	Point2DParameter* k2Anchor;

	virtual float getValue(const float& weight) override;

	void updateFromKeysInternal() override;
	void updateBezier();


	void onContainerParameterChanged(Parameter* p) override;

	/*class Bezier
	{
	public:
		Point<float> a;
		Point<float> b;
		Point<float> c;

		const float epsilon = 1e-6f; //Precision

		void setup(const Point<float>& a1, const Point<float>& a2);

		inline float sampleCurveX(float t);
		inline float sampleCurveY(float t);
		inline float sampleCurveDerivativeX(float t);

		float getValueForX(const float& tx);

		float solveCurveX(const float& tx);
	};
	*/

	Bezier::Bezier<3> bezier;
	Array<Point<float>> bezierLUT;

	//Array<float> solveCubic(float a, float b, float c, float d);

	EasingUI* createUI() override;
};


class SineEasing :
	public Easing
{
public:
	SineEasing(AutomationKey* k1, AutomationKey* k2, int dimension);
	Point2DParameter * freqAmp;

	virtual float getValue(const float &weight) override;

	EasingUI * createUI() override;
};

