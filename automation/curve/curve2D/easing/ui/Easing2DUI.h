/*
  ==============================================================================

	Easing2DUI.h
	Created: 16 Dec 2016 7:13:11pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

class Easing2DUI :
	public InspectableContentComponent,
	public ContainerAsyncListener
{
public:
	Easing2DUI(Easing2D* e);
	virtual ~Easing2DUI();

	juce::WeakReference<Easing2D> easing;

	juce::Path drawPath;
	juce::Path hitPath;

	bool showFirstHandle;
	bool showLastHandle;

	int hitPathPrecision = 10;
	juce::Rectangle<float> valueBounds;

	float focusPos = 0;
	float focusRange = 0;

	void paint(juce::Graphics& g) override;
	virtual void paintInternal(juce::Graphics&) {}
	void resized() override;

	void setFocus(float relPos, float range);

	void generatePath();
	virtual void generatePathInternal();

	void autoGeneratePathWithPrecision(int precision = 50);

	void buildHitPath();

	bool hitTest(int tx, int ty) override;

	virtual void setShowEasingHandles(bool showFirst, bool showLast);

	virtual void newMessage(const ContainerAsyncEvent& e) override;
	virtual void easingControllableFeedbackUpdate(Controllable*) {}

	void setValueBounds(const juce::Rectangle<float> valueBounds);
	juce::Point<int> getUIPosForValuePos(const juce::Point<float>& valuePos) const;
	juce::Point<float> getValuePosForUIPos(const juce::Point<int>& uiPos) const;

	class Easing2DHandle :
		public juce::Component
	{
	public:
		Easing2DHandle();
		void paint(juce::Graphics& g) override;
	};
};

class LinearEasing2DUI :
	public Easing2DUI
{
public:
	LinearEasing2DUI(LinearEasing2D* e);
	virtual ~LinearEasing2DUI() {}
	void generatePathInternal() override;

	// Inherited via Easing2DUI
};


class CubicEasing2DUI :
	public Easing2DUI
{
public:
	CubicEasing2DUI(CubicEasing2D* e);
	virtual ~CubicEasing2DUI() {}
	CubicEasing2D* ce;
	Easing2DHandle h1;
	Easing2DHandle h2;
	bool syncHandles;

	juce::Point<float> h1ValueAtMouseDown;
	juce::Point<float> h2ValueAtMouseDown;

	bool hitTest(int tx, int ty) override;

	void resized() override;

	void generatePathInternal() override;
	void paintInternal(juce::Graphics& g) override;

	void easingControllableFeedbackUpdate(Controllable*) override;

	void setShowEasingHandles(bool showFirst, bool showLast) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
};