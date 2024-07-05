/*
  ==============================================================================

	ParameterUI.h
	Created: 8 Mar 2016 3:48:44pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParameterUI :
	public ControllableUI,
	public Parameter::AsyncListener,
	public UITimerTarget	
{
public:
	ParameterUI(juce::Array<Parameter*> parameter, int paintTimerID = -1);
	virtual ~ParameterUI();

	juce::Array<juce::WeakReference<Parameter>> parameters;
	juce::WeakReference<Parameter> parameter;

	bool setUndoableValueOnMouseUp; //for slidable uis like floatSlider and floatLabelUIs
	bool showEditWindowOnDoubleClick;
	bool showValue;

	bool useCustomBGColor;
	juce::Colour customBGColor;
	bool useCustomFGColor;
	juce::Colour customFGColor;

	//popupMenuFilters
	static bool showAlwaysNotifyOption;
	static bool showControlModeOption;

	static std::function<void(ParameterUI*)> customShowEditRangeWindowFunction;


	virtual void showEditWindowInternal() override;
	virtual juce::Component* getEditValueComponent();

	void showEditRangeWindow();
	virtual void showEditRangeWindowInternal();

	void paintOverChildren(juce::Graphics& g) override;

	virtual void handlePaintTimer() override;
	virtual void handlePaintTimerInternal() override;

	virtual void addPopupMenuItems(juce::PopupMenu* p) override;
	virtual void addPopupMenuItemsInternal(juce::PopupMenu*) {}
	virtual void handleMenuSelectedID(int id) override;

	virtual void mouseDoubleClick(const juce::MouseEvent& e) override;
	virtual bool isInteractable(bool falseIfFeedbackOnly = true) override;

	//focus
	static int currentFocusOrderIndex;
	static void setNextFocusOrder(juce::Component* focusComponent);

	class ValueEditCalloutComponent :
		public juce::Component,
		public juce::Label::Listener
	{
	public:
		ValueEditCalloutComponent(juce::WeakReference<Parameter> pui);
		~ValueEditCalloutComponent();

		juce::WeakReference<Parameter> p;
		juce::OwnedArray<juce::Label> labels;

		void resized() override;
		void paint(juce::Graphics& g) override;
		void labelTextChanged(juce::Label* l) override;
		void editorHidden(juce::Label* l, juce::TextEditor&) override;
		void parentHierarchyChanged() override;
	};

	static double textToValue(const juce::String& text);

	juce::WeakReference<ParameterUI>::Master masterReference;

protected:

	virtual void visibilityChanged() override;

	// helper to spot wrong deletion order
	bool shouldBailOut();

	// here we are bound to only one parameter so no need to pass parameter*
	// for general behaviour see AsyncListener
	virtual void valueChanged(const juce::var&) {};
	virtual void rangeChanged(Parameter*) {};
	virtual void controlModeChanged(Parameter*);

	virtual void newMessage(const Parameter::ParameterEvent& e) override;;

};


