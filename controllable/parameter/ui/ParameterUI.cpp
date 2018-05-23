#include "ParameterUI.h"
/*
  ==============================================================================

	ParameterUI.cpp
	Created: 8 Mar 2016 3:48:44pm
	Author:  bkupe

  ==============================================================================
*/

//==============================================================================
ParameterUI::ParameterUI(Parameter * parameter) :
	ControllableUI(parameter),
	parameter(parameter),
    showEditWindowOnDoubleClick(true),
    showValue(true)
{
	parameter->addAsyncCoalescedParameterListener(this);
}

ParameterUI::~ParameterUI()
{
	if (!parameter.wasObjectDeleted()) {
		parameter->removeAsyncParameterListener(this);
	}
}

void ParameterUI::showEditWindow()
{
	AlertWindow nameWindow("Set a value", "Set a new value for this parameter", AlertWindow::AlertIconType::NoIcon, this);
	
	if(!parameter->isControllableFeedbackOnly) nameWindow.addTextEditor("val", parameter->stringValue(), "Value");

	if (parameter->isCustomizableByUser)
	{
		if (parameter->type == Parameter::FLOAT || parameter->type == Parameter::INT)
		{
			nameWindow.addTextEditor("minVal", parameter->minimumValue.toString(), "Minimum");
			nameWindow.addTextEditor("maxVal", parameter->maximumValue.toString(), "Maximum");
		}
	}

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		if (parameter->isCustomizableByUser)
		{
			if (parameter->type == Parameter::FLOAT || parameter->type == Parameter::INT)
			{
				float newMin = nameWindow.getTextEditorContents("minVal").getFloatValue();
				float newMax = nameWindow.getTextEditorContents("maxVal").getFloatValue();
				if(newMin < newMax) parameter->setRange(newMin, newMax);
			}
		}

		if (!parameter->isControllableFeedbackOnly)
		{
			float newValue = nameWindow.getTextEditorContents("val").getFloatValue();
			parameter->setUndoableValue(parameter->value, newValue);
		}
	}
}

void ParameterUI::paintOverChildren(Graphics & g)
{

	ControllableUI::paintOverChildren(g);
	if (parameter == nullptr) return;

	switch (parameter->controlMode)
	{
	case Parameter::EXPRESSION:
	{
		Colour c = LIGHTCONTOUR_COLOR;
		ScriptExpression::ExpressionState s = parameter->expression->state;
		if (s == ScriptExpression::EXPRESSION_ERROR) c = Colours::red.brighter(.3f);
		else if (s == ScriptExpression::EXPRESSION_LOADED) c = Colours::limegreen;

		g.setColour(c.withAlpha(.2f));
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 1);
	}
	break;

	case Parameter::REFERENCE:
	{
		Colour c = Colours::purple.brighter();
		g.setColour(c.withAlpha(.2f));
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 1);
	}
		break;

	case Parameter::AUTOMATION:
	{
		Colour c = BLUE_COLOR;
		g.setColour(c.withAlpha(.2f));
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 1);
	}
		break;
	}
}

void ParameterUI::addPopupMenuItems(PopupMenu * p)
{
	if (parameter->isEditable && (!parameter->isControllableFeedbackOnly || parameter->controlMode != Parameter::MANUAL))
	{
		p->addItem(1, "Reset value");

		if (parameter->isEditable && !parameter->isControllableFeedbackOnly && showEditWindowOnRightClick)
		{
			p->addSeparator();
			p->addItem(-3, "Show Edit Window");
			/*
			if (parameter->canHaveRange)
			{
				p->addItem(-4, "Set Range...");
				if((int)parameter->minimumValue > INT32_MIN || (int)parameter->maximumValue < INT32_MAX) p->addItem(-5, "Clear Range");
			}
			*/
			addPopupMenuItemsInternal(p);
		}

		p->addSeparator();
		if (!parameter->lockManualControlMode)
		{ 
			PopupMenu controlModeMenu;
			controlModeMenu.addItem(10, "Manual");
			controlModeMenu.addItem(11, "Expression");
			controlModeMenu.addItem(12, "Reference");
			controlModeMenu.addItem(13, "Animation");
			p->addSubMenu("Control Mode", controlModeMenu);
		}
	}
}

void ParameterUI::handleMenuSelectedID(int id)
{
	switch (id)
	{
	case 1: parameter->resetValue(); break;
	case 10: parameter->setControlMode(Parameter::MANUAL); break;
	case 11: parameter->setControlMode(Parameter::EXPRESSION); break;
	case 12: parameter->setControlMode(Parameter::REFERENCE); break;
	case 13: parameter->setControlMode(Parameter::AUTOMATION); break;
	}
}

void ParameterUI::mouseDoubleClick(const MouseEvent & e)
{
	if (showEditWindowOnDoubleClick) showEditWindow();
}



bool ParameterUI::shouldBailOut() {
	bool bailOut = parameter.get() == nullptr;
	// we want a clean deletion no?
	jassert(!bailOut);
	return bailOut;

}

// see Parameter::AsyncListener

void ParameterUI::newMessage(const Parameter::ParameterEvent &e) {
	switch(e.type)
	{
	case Parameter::ParameterEvent::BOUNDS_CHANGED:
		rangeChanged(e.parameter);
		break;

	case Parameter::ParameterEvent::VALUE_CHANGED:
		valueChanged(e.value);
		break;

	case Parameter::ParameterEvent::EXPRESSION_STATE_CHANGED:
	case Parameter::ParameterEvent::CONTROLMODE_CHANGED:
		repaint();
		break;
	}
}
