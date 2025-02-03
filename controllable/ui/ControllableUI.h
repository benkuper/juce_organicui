/*
  ==============================================================================

    ControllableUI.h
    Created: 9 Mar 2016 12:02:16pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ControllableUI :
	public juce::Component,
	public juce::SettableTooltipClient,
	public Controllable::AsyncListener,
	public Inspectable::AsyncListener
{
public:
	ControllableUI(juce::Array<Controllable*> controllables);
	virtual ~ControllableUI();

	static bool drawContourOnInspectableHighlighted;

	juce::String tooltip;
	juce::Array<juce::WeakReference<Controllable>>  controllables;
	juce::WeakReference<Controllable> controllable; //convenience access to the first item, which a lot of the UI is based upon

	static std::function<void(ControllableUI*)> customShowContextMenuFunc;
	static std::function<void(ControllableUI*, juce::PopupMenu*) > customAddToContextMenuFunc;
	static std::function<bool(ControllableUI*, int)> handleCustomContextMenuResultFunc;

	bool showLabel;
	bool opaqueBackground;
	bool showMenuOnRightClick;

	bool forceFeedbackOnly;

	bool useCustomTextColor;
	juce::Colour customTextColor;
	int customTextSize;

	bool useCustomContour;
	juce::Colour customContourColor;
	float customContourThickness;

	juce::String customLabel;
	juce::String customDescription;

	//popupmenu filters
	static bool showOSCControlAddressOption;
	static bool showScriptControlAddressOption;
	static bool showDetectiveOption;
	static bool showDashboardOption;
	static bool showParrotOption;

	std::function<void(ControllableUI* i, Inspectable* source)> customHighlightFunc;

	static std::function<void(ControllableUI*)> customShowEditWindowFunction;

	virtual void paintOverChildren(juce::Graphics& g) override;

	void mouseEnter(const juce::MouseEvent& e) override;
	void mouseExit(const juce::MouseEvent& e) override;
	void mouseDown(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	virtual void drawContour(juce::Graphics& g);

	virtual void mouseDownInternal(const juce::MouseEvent&) {} //avoid right click check in each child class
	virtual void mouseUpInternal(const juce::MouseEvent&) {} //

	virtual bool isInteractable(bool falseIfFeedbackOnly = true); //Helper for checking if this ui can be interacted with

	//PopupMenu
	virtual void showContextMenu();
	virtual void addPopupMenuItems(juce::PopupMenu*) {} //for child classes
	virtual void handleMenuSelectedID(int) {} // to override

	virtual void updatePreview() {}

	virtual void updateUIParams(); //to override to update colors and stuff depending on custom flags
	virtual void updateUIParamsInternal() {} //to override to update colors and stuff depending on custom flags

	virtual void setOpaqueBackground(bool value);

	void showEditWindow();
	virtual void showEditWindowInternal() {} //to be overriden by children

	virtual void newMessage(const Controllable::ControllableEvent &e) override;
	virtual void newMessage(const Inspectable::InspectableEvent& e) override;

	virtual void controllableStateChanged();
	virtual void feedbackStateChanged();
	virtual void controllableControlAddressChanged() {}

	virtual void updateTooltip();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableUI)

};

