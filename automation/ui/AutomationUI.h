/*
  ==============================================================================

	AutomationUI.h
	Created: 11 Dec 2016 1:22:02pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationUILayer :
	public juce::Component
{
public:
	AutomationUILayer(AutomationUI* _ui, int _id);
	~AutomationUILayer() {}

	AutomationUI* ui;
	int id;

	void paint(juce::Graphics& g) override;
	void resized() override;
};

class AutomationUI;

class AutomationUIKeys :
	public ManagerUI<Automation, AutomationKey>,
	public AutomationKey::AsyncListener,
	public AutomationKeyUI::KeyUIListener,
	public ContainerAsyncListener,
	public Automation::AsyncListener
{
public:
	AutomationUIKeys(Automation* manager, AutomationUI* _autoUI);
	~AutomationUIKeys();

	AutomationUI* autoUI;

	juce::Point<float> viewPosRange;
	float viewLength;
	bool autoAdaptViewRange;

	bool paintingMode;
	juce::Array<juce::Point<float>> paintingPoints;
	juce::Point<float> lastPaintingPoint;

	bool previewMode; //avoid repainting everything

	juce::Point<float> viewValueRangeAtMouseDown;

	std::unique_ptr<FloatSliderUI> interactiveSimplificationUI;
	std::unique_ptr<juce::TextButton> validInteractiveBT;

	juce::Array<float> snapTimes;

	void paint(juce::Graphics& g) override;
	void drawLinesBackground(juce::Graphics& g);
	void paintOverlay(juce::Graphics& g);
	void paintBackground(juce::Graphics& g);

	void resized() override;
	void placeKeyUI(AutomationKeyUI* ui);
	void updateHandlesForUI(AutomationKeyUI* ui, bool checkSideItems);

	void setPreviewMode(bool value);

	void setViewRange(float start, float end);
	void updateItemsVisibility() override;

	void addItemUIInternal(BaseItemMinimalUI* ui) override;
	void removeItemUIInternal(BaseItemMinimalUI* ui) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
	void mouseDoubleClick(const juce::MouseEvent& e) override;

	void addItemFromMenu(BaseItem* item, bool fromAddbutton, juce::Point<int> pos) override;

	void addMenuExtraItems(juce::PopupMenu& p, int startIndex) override;
	void handleMenuExtraItemsResult(int result, int startIndex) override;

	juce::Component* getSelectableComponentForItemUI(BaseItemMinimalUI* ui) override;

	juce::Point<float> getViewPos(juce::Point<int> pos, bool relative = false);
	juce::Rectangle<float> getViewBounds(juce::Rectangle<int> pos, bool relative = false);
	juce::Point<int> getPosInView(juce::Point<float> pos, bool relative = false);
	juce::Rectangle<int> getBoundsInView(juce::Rectangle<float> pos, bool relative = false);

	float getPosForX(int x, bool relative = false);
	int getXForPos(float x, bool relative = false);
	float getValueForY(int y, bool relative = false);
	int getYForValue(float x, bool relative = false);

	void newMessage(const AutomationKey::AutomationKeyEvent& e) override;
	void newMessage(const ContainerAsyncEvent& e) override;
	void newMessage(const Automation::AutomationEvent& e) override;

	void keyEasingHandleMoved(AutomationKeyUI* ui, bool syncOtherHandle, bool isFirst) override;

	void buttonClicked(juce::Button* b) override;
};

class AutomationUI :
	public juce::Component,
	public UITimerTarget
{
public:
	AutomationUI(Automation* manager);
	~AutomationUI() {}

	AutomationUIKeys keysUI;
	AutomationUILayer cursor;
	AutomationUILayer overlay;
	AutomationUILayer background;

	bool shouldRepaintKeys;
	bool shouldRepaintOverlay;
	bool shouldResize;

	bool disableOverlayFill;

	juce::Point<int> lastRepaintOverlayPoint;
	int overlayStartY;

	bool showNumberLines;
	bool showMenuOnRightClick;
	std::function<void(juce::Array<float>*, AutomationKey* k)> getSnapTimesFunc;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setRepaint(bool keys, bool overlay, bool background);

	void paintOverChildren(juce::Graphics& g) override;
	virtual void handlePaintTimerInternal() override;

	inline void setPreviewMode(bool value) { keysUI.setPreviewMode(value); }
	inline void setViewRange(float start, float end) { keysUI.setViewRange(start, end); }

	void addSelectableComponentsAndInspectables(juce::Array<juce::Component*>& selectables, juce::Array<Inspectable*>& inspectables) { keysUI.addSelectableComponentsAndInspectables(selectables, inspectables); }
};
