/*
  ==============================================================================

    Easing2D.h
    Created: 21 Mar 2020 4:06:44pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../../../common/bezier.h"

class Easing2DUI;

class Easing2D :
	public ControllableContainer
{
public:
	enum Type { LINEAR, BEZIER };

	Easing2D(int type);
	virtual ~Easing2D();

	int type; //must be int to be able to extend
	Point<float> start;
	Point<float> end;
	float length;

	virtual void updateKeys(const Point<float>& start, const Point<float>& end, bool updateLength = true);

	virtual Point<float> getValue(const float& weight) = 0;//must be overriden
	virtual void updateLength() = 0;
	virtual Rectangle<float> getBounds(bool includeHandles = false) = 0;
	virtual Point<float> getClosestPointForPos(Point<float> pos) = 0;
	virtual Easing2DUI* createUI() = 0; //must be overriden

private:
	WeakReference<Easing2D>::Master masterReference;
	friend class WeakReference<Easing2D>;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Easing2D)

};

class LinearEasing2D :
	public Easing2D
{
public:
	LinearEasing2D();
	virtual ~LinearEasing2D() {}
	Point<float> getValue(const float& weight) override;
	void updateLength() override;
	Rectangle<float> getBounds(bool includeHandles) override;
	Point<float> getClosestPointForPos(Point<float> pos) override;

	Easing2DUI* createUI() override;
};


class CubicEasing2D :
	public Easing2D
{
public:
	CubicEasing2D();
	virtual ~CubicEasing2D() {}
	Point2DParameter* anchor1;
	Point2DParameter* anchor2;

	Bezier::Bezier<3> bezier;
	Array<Point<float>, CriticalSection> uniformLUT;

	void updateKeys(const Point<float>& start, const Point<float>& end, bool updateKeys = true) override;
	void updateBezier();
	void updateUniformLUT(int precision);

	Point<float> getValue(const float& weight) override;
	Point<float> getRawValue(const float& weight);

	void updateLength() override;
	void getBezierLength(Point<float> a, Point<float> b, Point<float> c, Point<float> d, int precision, float& length);
	
	Rectangle<float> getBounds(bool includeHandles) override;
    Point<float> getClosestPointForPos(Point<float> pos) override;

	void onContainerParameterChanged(Parameter* p) override;

	Easing2DUI* createUI() override;
};
