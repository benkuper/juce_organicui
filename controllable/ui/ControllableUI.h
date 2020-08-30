/*
  ==============================================================================

    ControllableUI.h
    Created: 9 Mar 2016 12:02:16pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ControllableUI :
	public Component,
	public SettableTooltipClient,
	public Controllable::AsyncListener
{
public:
	ControllableUI(Controllable* controllable);
	virtual ~ControllableUI();

	String tooltip;
	WeakReference<Controllable>  controllable;

	static std::function<void(ControllableUI*)> customShowContextMenuFunc;
	static std::function<void(ControllableUI*, PopupMenu*)> customAddToContextMenuFunc;
	static std::function<bool(ControllableUI*, int)> handleCustomContextMenuResultFunc;

	bool showLabel;
	bool opaqueBackground;
	bool showMenuOnRightClick;

	bool useCustomTextColor;
	Colour customTextColor;

	bool useCustomContour;
	Colour customContourColor;
	float customContourThickness;

	String customLabel;
	String customDescription;

	//popupmenu filters
	static bool showOSCControlAddressOption;
	static bool showScriptControlAddressOption;
	static bool showDetectiveOption;
	static bool showDashboardOption;

	static std::function<void(ControllableUI*)> customShowEditWindowFunction;

	virtual void paintOverChildren(Graphics& g) override;

	void mouseEnter(const MouseEvent& e) override;
	void mouseExit(const MouseEvent& e) override;
	void mouseDown(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;

	virtual void drawContour(Graphics& g);

	virtual void mouseDownInternal(const MouseEvent&) {} //avoid right click check in each child class
	virtual void mouseUpInternal(const MouseEvent&) {} //

	virtual bool isInteractable(); //Helper for checking if this ui can be interacted with

	//PopupMenu
	virtual void showContextMenu();
	virtual void addPopupMenuItems(PopupMenu*) {} //for child classes
	virtual void handleMenuSelectedID(int) {} // to override

	virtual void updateUIParams(); //to override to update colors and stuff depending on custom flags
	virtual void updateUIParamsInternal() {} //to override to update colors and stuff depending on custom flags

	virtual void setOpaqueBackground(bool value);

	void showEditWindow();
	virtual void showEditWindowInternal() {} //to be overriden by children

	virtual void newMessage(const Controllable::ControllableEvent &e) override;

	virtual void controllableStateChanged();
	virtual void feedbackStateChanged();
	virtual void controllableControlAddressChanged() {}

	virtual void updateTooltip();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableUI)

};

