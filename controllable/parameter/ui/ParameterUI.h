/*
  ==============================================================================

    ParameterUI.h
    Created: 8 Mar 2016 3:48:44pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef PARAMETERUI_H_INCLUDED
#define PARAMETERUI_H_INCLUDED

#include "../Parameter.h"

class ParameterUI :
	public ControllableUI,
	public Parameter::AsyncListener
{
public:
    ParameterUI(Parameter * parameter);
    virtual ~ParameterUI();

	
    WeakReference<Parameter> parameter;

	bool showEditWindowOnDoubleClick;
	bool showEditWindowOnRightClick;

	bool showValue;
	void showEditWindow() override;

	void paintOverChildren(Graphics &g) override;

	virtual void addPopupMenuItems(PopupMenu * p) override;
	virtual void handleMenuSelectedID(int id) override;

	virtual void mouseDoubleClick(const MouseEvent &e) override;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterUI)
};


#endif  // PARAMETERUI_H_INCLUDED
