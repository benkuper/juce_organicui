/*
  ==============================================================================

	ParameterUI.h
	Created: 8 Mar 2016 3:48:44pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#define PARAMETERUI_DEFAULT_TIMER 0
#define PARAMETERUI_SLOW_TIMER 1



class ParameterUI :
	public ControllableUI,
	public Parameter::AsyncListener
{
public:
	ParameterUI(Array<Parameter*> parameter, int paintTimerID = -1);
	virtual ~ParameterUI();

	Array<WeakReference<Parameter>> parameters;
	WeakReference<Parameter> parameter;

	//painting
	int paintTimerID;
	bool shouldRepaint;

	bool setUndoableValueOnMouseUp; //for slidable uis like floatSlider and floatLabelUIs
	bool showEditWindowOnDoubleClick;
	bool showValue;

	bool useCustomBGColor;
	Colour customBGColor;
	bool useCustomFGColor;
	Colour customFGColor;

	//popupMenuFilters
	static bool showAlwaysNotifyOption;
	static bool showControlModeOption;

	static std::function<void(ParameterUI*)> customShowEditRangeWindowFunction;


	virtual void showEditWindowInternal() override;
	virtual Component* getEditValueComponent();

	void showEditRangeWindow();
	virtual void showEditRangeWindowInternal();

	void paintOverChildren(Graphics& g) override;

	virtual void handlePaintTimer();
	virtual void handlePaintTimerInternal();

	virtual void addPopupMenuItems(PopupMenu* p) override;
	virtual void addPopupMenuItemsInternal(PopupMenu*) {}
	virtual void handleMenuSelectedID(int id) override;

	virtual void mouseDoubleClick(const MouseEvent& e) override;
	virtual bool isInteractable(bool falseIfFeedbackOnly = true) override;

	//focus
	static int currentFocusOrderIndex;
	static void setNextFocusOrder(Component* focusComponent);

	class ValueEditCalloutComponent :
		public Component,
		public Label::Listener
	{
	public:
		ValueEditCalloutComponent(WeakReference<Parameter> pui);
		~ValueEditCalloutComponent();

		WeakReference<Parameter> p;
		OwnedArray<Label> labels;

		void resized() override;
		void paint(Graphics& g) override;
		void labelTextChanged(Label* l) override;
		void editorHidden(Label* l, TextEditor&) override;
		void parentHierarchyChanged() override;
	};

	static double textToValue(const String& text);

	WeakReference<ParameterUI>::Master masterReference;

protected:

	virtual void visibilityChanged() override;

	// helper to spot wrong deletion order
	bool shouldBailOut();

	// here we are bound to only one parameter so no need to pass parameter*
	// for general behaviour see AsyncListener
	virtual void valueChanged(const var&) {};
	virtual void rangeChanged(Parameter*) {};
	virtual void controlModeChanged(Parameter*);

	virtual void newMessage(const Parameter::ParameterEvent& e) override;;

};


class ParameterUITimers :
	public MultiTimer
{
public:
	juce_DeclareSingleton(ParameterUITimers, true);
	ParameterUITimers();
	~ParameterUITimers() {}

	HashMap<int, Array<WeakReference<ParameterUI>>> paramsTimerMap;

	void registerParameter(int timerID, ParameterUI* ui);
	void unregisterParameter(int timerID, ParameterUI* ui);
	void timerCallback(int timerID);
};