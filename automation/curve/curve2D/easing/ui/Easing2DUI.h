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

	WeakReference<Easing2D> easing;

	Path drawPath;
	Path hitPath;

	bool showFirstHandle;
	bool showLastHandle;

	int hitPathPrecision = 10;

	Rectangle<float> valueBounds;

	void paint(Graphics& g) override;
	virtual void paintInternal(Graphics&) {}
	void resized() override;

	void generatePath();
	virtual void generatePathInternal();

	void autoGeneratePathWithPrecision(int precision = 50);

	void buildHitPath();

	bool hitTest(int tx, int ty) override;

	virtual void setShowEasingHandles(bool showFirst, bool showLast);

	virtual void newMessage(const ContainerAsyncEvent& e) override;
	virtual void easingControllableFeedbackUpdate(Controllable*) {}

	void setValueBounds(const Rectangle<float> valueBounds);
	Point<int> getUIPosForValuePos(const Point<float>& valuePos) const;
	Point<float> getValuePosForUIPos(const Point<int>& uiPos) const;

	class Easing2DHandle :
		public Component
	{
	public:
		Easing2DHandle();
		void paint(Graphics& g) override;
	};
};

class LinearEasing2DUI :
	public Easing2DUI
{
public:
	LinearEasing2DUI(LinearEasing2D* e);
	void generatePathInternal() override;

	// Inherited via Easing2DUI
};


class CubicEasing2DUI :
	public Easing2DUI
{
public:
	CubicEasing2DUI(CubicEasing2D* e);

	CubicEasing2D* ce;
	Easing2DHandle h1;
	Easing2DHandle h2;
	bool syncHandles;

	Point<float> h1ValueAtMouseDown;
	Point<float> h2ValueAtMouseDown;

	bool hitTest(int tx, int ty) override;

	void resized() override;

	void generatePathInternal() override;
	void paintInternal(Graphics& g) override;

	void easingControllableFeedbackUpdate(Controllable*) override;

	void setShowEasingHandles(bool showFirst, bool showLast) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
};