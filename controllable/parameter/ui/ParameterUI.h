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
	public Parameter::AsyncListener
{
public:
    ParameterUI(Parameter * parameter);
    virtual ~ParameterUI();
	
    WeakReference<Parameter> parameter;

	bool showEditWindowOnDoubleClick;
	bool showValue;

	static std::function<void(ParameterUI*)> customShowEditRangeWindowFunction;


	virtual void showEditWindowInternal() override;
	void showEditRangeWindow();
	virtual void showEditRangeWindowInternal();

	void paintOverChildren(Graphics &g) override;

	virtual void addPopupMenuItems(PopupMenu * p) override;
	virtual void addPopupMenuItemsInternal(PopupMenu *) {}
	virtual void handleMenuSelectedID(int id) override;

	virtual void mouseDoubleClick(const MouseEvent &e) override;

	virtual bool isInteractable() override;

	//focus
	static int currentFocusOrderIndex;
	static void setNextFocusOrder(Component * focusComponent);

protected:


    // helper to spot wrong deletion order
    bool shouldBailOut();

    // here we are bound to only one parameter so no need to pass parameter*
    // for general behaviour see AsyncListener
    virtual void valueChanged(const var & ){};
	virtual void rangeChanged(Parameter * ){};


protected:
    // see Parameter::AsyncListener
	virtual void newMessage(const Parameter::ParameterEvent &e) override;;
};

