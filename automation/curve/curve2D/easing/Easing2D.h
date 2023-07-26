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
	juce::Point<float> start;
	juce::Point<float> end;
	float length;

	virtual void updateKeys(const juce::Point<float>& start, const juce::Point<float>& end, bool updateLength = true);

	virtual juce::Point<float> getValue(const float& weight) = 0;//must be overriden
	virtual void updateLength() = 0;
	virtual juce::Rectangle<float> getBounds(bool includeHandles = false) = 0;
	virtual juce::Point<float> getClosestPointForPos(juce::Point<float> pos) = 0;
	virtual Easing2DUI* createUI() = 0; //must be overriden

private:
	juce::WeakReference<Easing2D>::Master masterReference;
	friend class juce::WeakReference<Easing2D>;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Easing2D)

};

class LinearEasing2D :
	public Easing2D
{
public:
	LinearEasing2D();
	virtual ~LinearEasing2D() {}
	juce::Point<float> getValue(const float& weight) override;
	void updateLength() override;
	juce::Rectangle<float> getBounds(bool includeHandles) override;
	juce::Point<float> getClosestPointForPos(juce::Point<float> pos) override;

	
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
	juce::Array<juce::Point<float>, juce::CriticalSection> uniformLUT;

	void updateKeys(const juce::Point<float>& start, const juce::Point<float>& end, bool updateKeys = true) override;
	void updateBezier();
	void updateUniformLUT(int precision);

	juce::Point<float> getValue(const float& weight) override;
	juce::Point<float> getRawValue(const float& weight);

	void updateLength() override;
	void getBezierLength(juce::Point<float> a, juce::Point<float> b, juce::Point<float> c, juce::Point<float> d, int precision, float& length);
	
	juce::Array<juce::Point<float>> getSplitControlPoints(const juce::Point<float> & pos);

	juce::Rectangle<float> getBounds(bool includeHandles) override;

	juce::Point<float> getClosestPointForPos(juce::Point<float> pos) override;
	float getBezierWeight(const juce::Point<float>& pos);

	void onContainerParameterChanged(Parameter* p) override;

	Easing2DUI* createUI() override;
};
