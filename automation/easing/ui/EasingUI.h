/*
  ==============================================================================

    EasingUI.h
    Created: 16 Dec 2016 7:13:11pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class EasingUI :
	public InspectableContentComponent,
	public ContainerAsyncListener
{
public:
	EasingUI(Easing * e, Colour color = Colours::white);
	virtual ~EasingUI();

	WeakReference<Easing> easing;

	Colour color;

	Point<float> viewValueRange;
	Point<int> p1;
	Point<int> p2;

	Path drawPath;
	Path hitPath;

	int hitPathPrecision = 10;

	void setViewValueRange(const Point<float> range, bool updatePath = true);

	void paint(Graphics &g) override;
	virtual void paintInternal(Graphics &) {}
	void resized() override;

	void generatePath();
	virtual void generatePathInternal();

	void autoGeneratePathWithPrecision(int precision = 100);

	void buildHitPath();

	int getYForValue(float value);
	float getValueForY(int y);

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
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasingUI)
};

class LinearEasingUI :
	public EasingUI
{
public:
	LinearEasingUI(LinearEasing * e);
	void generatePathInternal() override;

	// Inherited via EasingUI
};


class HoldEasingUI :
	public EasingUI
{
public:
	HoldEasingUI(HoldEasing * e);
	void generatePathInternal() override;
};


class CubicEasingUI :
	public EasingUI
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
	public EasingUI
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