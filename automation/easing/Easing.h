/*
  ==============================================================================

    Easing.h
    Created: 11 Dec 2016 1:29:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class EasingUI;
#include "../common/bezier.h"

class Easing :
	public ControllableContainer
{
public:
	enum Type { LINEAR, BEZIER, HOLD, SINE, ELASTIC, BOUNCE, STEPS, NOISE, PERLIN, TYPE_MAX };
	static const String typeNames[TYPE_MAX];

	Easing(Type type);
	virtual ~Easing();

	Type type;
	Point<float> start;
	Point<float> end;
	float prevLength;
	float length;

	virtual void updateKeys(const Point<float>& start, const Point<float>& end, bool stretch = false);
	virtual void updateKeysInternal(bool stretch = false) {}

	virtual float getValue(const float& weight) = 0;//must be overriden
	virtual Rectangle<float> getBounds(bool includeHandles = false) = 0;
	virtual EasingUI* createUI();

	float getWeightForPos(float pos);
	Point<float> getClosestPointForPos(float pos);


private:
	WeakReference<Easing>::Master masterReference;
	friend class WeakReference<Easing>;
};

class LinearEasing :
	public Easing
{
public:
	LinearEasing();

	float getValue(const float& weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI* createUI() override;
};

class HoldEasing :
	public Easing
{
public:
	HoldEasing();

	virtual float getValue(const float& weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI* createUI() override;
};

class CubicEasing :
	public Easing
{
public:
	CubicEasing();
	Point2DParameter* anchor1;
	Point2DParameter* anchor2;

	//for generating timeLUT
	Point<float> a;
	Point<float> b;
	Point<float> c;

	virtual float getValue(const float& weight) override;
	Point<float> getRawValue(const float &weight);

	float getBezierWeight(const float& pos);

	void updateKeysInternal(bool stretch = false) override;
	void updateBezier();

	void updateUniformLUT(int precision);

	void onContainerParameterChanged(Parameter* p) override;

	Bezier::Bezier<3> bezier;
	Array<float> uniformLUT;

	Array<Point<float>> getSplitControlPoints(float pos);

	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI* createUI() override;
};


class SineEasing :
	public Easing
{
public:
	SineEasing();
	Point2DParameter * freqAmp;

	void updateKeysInternal(bool stretch = false) override;

	virtual float getValue(const float &weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI * createUI() override;
};

class ElasticEasing :
	public Easing
{
public:
	ElasticEasing();
	Point2DParameter* param;

	void updateKeysInternal(bool stretch = false) override;

	virtual float getValue(const float& weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI* createUI() override;
};


class BounceEasing :
	public Easing
{
public:
	BounceEasing();

	void updateKeysInternal(bool stretch = false) override;

	virtual float getValue(const float& weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;
	EasingUI* createUI() override;
};


class StepEasing :
	public Easing
{
public:
	StepEasing();
	Point2DParameter* param;

	void updateKeysInternal(bool stretch = false) override;

	virtual float getValue(const float& weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI* createUI() override;
};

class NoiseEasing:
	public Easing
{
public:
	NoiseEasing();
	Point2DParameter* taper1;
	Point2DParameter* taper2;
	Random r;

	void updateKeysInternal(bool stretch = false) override;

	virtual float getValue(const float& weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI* createUI() override;
};

class PerlinEasing :
	public Easing
{
public:
	PerlinEasing();
	Point2DParameter* taper1;
	Point2DParameter* taper2;
	siv::PerlinNoise perlin;

	void updateKeysInternal(bool stretch = false) override;

	virtual float getValue(const float& weight) override;
	Rectangle<float> getBounds(bool includeHandles) override;

	EasingUI* createUI() override;
};

