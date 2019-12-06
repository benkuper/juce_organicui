/*
  ==============================================================================

    EasingUI.h
    Created: 16 Dec 2016 7:13:11pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class EasingTimelineUIBase :
	public InspectableContentComponent,
	public ContainerAsyncListener
{
public:
	EasingTimelineUIBase(EasingBase* e);
	virtual ~EasingTimelineUIBase();

	WeakReference<EasingBase> easingBase;

	Colour color;

	int numDimensions;

	OwnedArray<Path> drawPaths;
	OwnedArray<Path> hitPaths;
	Array<int> y1;
	Array<int> y2;

	int hitPathPrecision = 10;

	void setupDimensions(int num);

	void paint(Graphics &g) override;
	virtual void paintInternal(Graphics &) {}
	void resized() override;

	virtual void generatePaths();
	virtual void generatePathsInternal(int index);
	virtual int getYValue(int index) = 0;
	virtual void autoGeneratePathWithPrecision(int index, int precision = 100) = 0;

	void buildHitPaths(bool isY1, int index);

    bool hitTest(int tx, int ty) override;

	virtual void newMessage(const ContainerAsyncEvent &e) override;
	virtual void easingControllableFeedbackUpdate(Controllable *) {}

	class EasingHandle :
		public Component
	{
	public :
		EasingHandle();
		void paint(Graphics &g) override;
	};

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasingTimelineUIBase)
};


template<class T>
class EasingTimelineUI
{
public:
	EasingTimelineUI(Easing<T>* easing);
	virtual ~EasingTimelineUI() {}

	Easing<T>* easing;

	int getYValue(bool isY1, int index);
	void setKeyPositions(const T& k1, const T& k2);
	void autoGeneratePathWithPrecision(int index, int precision = 100) override;

};


template<class T>
EasingTimelineUI<T>::EasingTimelineUI(Easing<T>* easing) : 
	EasingTimelineUIBase(easing),
	easing(easing)
{
	int numDimensions = 0;
	if (std::is_same(T, float)) numDimensions = 1;
	else if (std::is_same(T, Point<float>)) numDimensions = 2;
	else if (std::is_same(T, Vector3D<float>))  numDimensions = 3;

	setupDimensions(numDimensions);
}

template<class T>
void EasingTimelineUI<T>::setKeyPositions(const T& k1, const T& k2)
{
	if (std::is_same<T, float>)
	{
		y1[0] = k1;
		y2[0] = k2;
	}
	else if (std::is_same<T, Point2D<float>>)
	{
		y1[0] = k1.x;
		y2[0] = k2.x;
		y1[1] = k1.y;
		y2[1] = k2.y;
	}
	else if (std::is_same<T, Vector3D<float>>)
	{
		y1[0] = k1.x;
		y2[0] = k2.x; 
		y1[1] = k1.y;
		y2[1] = k2.y;
		y1[2] = k1.z;
		y2[2] = k2.z;
	}

	generatePaths();
}

template<class T>
void EasingTimelineUI<T>::autoGeneratePathWithPrecision(int index, int precision)
{
	if (getHeight() == 0) return;
	if (precision == 0) precision = getWidth();
	else precision = jmin(getWidth(), precision);

	for (int i = 1; i <= precision; i++)
	{
		float t = i * 1.f / precision;
		float v1 = 1 - (y1 * 1.f / getHeight());
		float v2 = 1 - (y2 * 1.f / getHeight());
		
		T tv = easingBase->getValue(v1, v2, t);
		float v = 0;
		if (std::is_same<T, float>) v = tv;
		else if (std::is_same<T, Point<float>>) v = ((Point<float>)(tv))[i];
		else if (std::is_same<T, Vector3D<float>>) v = ((Vector3D<float>)(tv))[i];

		drawPath.lineTo(t * getWidth(), (1 - v) * getHeight());
	}
}


template<class T>
class LinearEasingTimelineUI :
	public EasingTimelineUI<T>
{
public:
	LinearEasingUI(LinearEasing<T> * e);
	void generatePathInternal() override;

	// Inherited via EasingUI
};


template<class T>
class HoldEasingTimelineUI :
	public EasingTimelineUI<T>
{
public:
	HoldEasingUI(HoldEasing<T> * e);
	void generatePathInternal() override;
};


/*
class CubicEasingUI :
	public EasingTimelineUI<T>
{
public:
	CubicEasingUI(CubicEasing * e);
	
	EasingHandle h1;
	EasingHandle h2;


	bool hitTest(int tx, int ty) override;

	void resized() override;

	void generatePathInternal() override;
	void paintInternal(Graphics &g) override;

	void inspectableSelectionChanged(Inspectable *) override;
	void easingControllableFeedbackUpdate(Controllable *) override;

	void mouseDrag(const MouseEvent &e) override;
};


class SineEasingUI :
	public EasingTimelineUI<T>
{
public:
	SineEasingUI(SineEasing * e);

	EasingHandle h1;

	bool hitTest(int tx, int ty) override;

	void resized() override;

	void paintInternal(Graphics &g) override;

	void inspectableSelectionChanged(Inspectable *) override;
	void easingControllableFeedbackUpdate(Controllable *) override;

	void mouseDrag(const MouseEvent &e) override;
};
*/