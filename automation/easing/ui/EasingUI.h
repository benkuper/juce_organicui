#pragma once


class EasingUI :
	public InspectableContentComponent,
	public ContainerAsyncListener,
	public UITimerTarget
{
public:
	EasingUI(Easing* e);
	virtual ~EasingUI();

	juce::WeakReference<Easing> easing;

	juce::Path drawPath;
	juce::Path hitPath;

	bool showFirstHandle;
	bool showLastHandle;

	int hitPathPrecision = 10;

	juce::Rectangle<float> valueBounds;

	virtual void handlePaintTimerInternal() override;
	void paint(juce::Graphics& g) override;
	virtual void paintInternal(juce::Graphics&) {}
	void resized() override;

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

	class EasingHandle :
		public juce::Component
	{
	public:
		EasingHandle(Point2DParameter * parameter);
		Point2DParameter* parameter;

		void mouseDoubleClick(const juce::MouseEvent &e) override;
		void paint(juce::Graphics& g) override;
	};

	class HandleEditCalloutComponent :
		public juce::Component
	{
	public:
		HandleEditCalloutComponent(Point2DParameter * p);
		~HandleEditCalloutComponent();

		AutomationKey* k;
		std::unique_ptr<DoubleSliderUI> paramEditor;

		void resized() override;
	};
};

class LinearEasingUI :
	public EasingUI
{
public:
	LinearEasingUI(LinearEasing* e);
	void generatePathInternal() override;

	// Inherited via EasingUI
};


class CubicEasingUI :
	public EasingUI
{
public:
	CubicEasingUI(CubicEasing* e);

	CubicEasing* ce;
	EasingHandle h1;
	EasingHandle h2;
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

class HoldEasingUI :
	public EasingUI
{
public:
	HoldEasingUI(HoldEasing* e);
	void generatePathInternal() override;

	// Inherited via EasingUI
};

class SineEasingUI :
	public EasingUI
{
public:
	SineEasingUI(SineEasing* e);

	SineEasing* se;

	EasingHandle h1;
	juce::Point<float> h1ValueAtMouseDown;

	bool hitTest(int tx, int ty) override;

	void resized() override;

	void paintInternal(juce::Graphics& g) override;

	void easingControllableFeedbackUpdate(Controllable*) override;

	void setShowEasingHandles(bool showFirst, bool showLast) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
};

class ElasticEasingUI :
	public EasingUI
{
public:
	ElasticEasingUI(ElasticEasing* e);

	ElasticEasing* se;

	EasingHandle h1;
	juce::Point<float> h1ValueAtMouseDown;

	bool hitTest(int tx, int ty) override;

	void resized() override;

	void paintInternal(juce::Graphics& g) override;

	void easingControllableFeedbackUpdate(Controllable*) override;

	void setShowEasingHandles(bool showFirst, bool showLast) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
};

class BounceEasingUI :
	public EasingUI
{
public:
	BounceEasingUI(BounceEasing* e);

	BounceEasing* se;

	void resized() override;
	void paintInternal(juce::Graphics& g) override;
};

class GenericEasingUI :
	public EasingUI
{
public:
	GenericEasingUI(Easing* e, Point2DParameter* a1 = nullptr, Point2DParameter* a2 = nullptr, juce::Array<Parameter*> extraParams = juce::Array<Parameter *>());
	~GenericEasingUI() {}

	std::unique_ptr<EasingHandle> h1;
	std::unique_ptr<EasingHandle> h2;
	juce::OwnedArray<ControllableUI> extraParams;

	juce::Point<float> h1ValueAtMouseDown;
	juce::Point<float> h2ValueAtMouseDown;

	bool hitTest(int tx, int ty) override;

	void resized() override;

	void paintInternal(juce::Graphics& g) override;

	void easingControllableFeedbackUpdate(Controllable*) override;

	void setShowEasingHandles(bool showFirst, bool showLast) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
};