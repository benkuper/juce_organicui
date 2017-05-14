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
	showValue(true)
{
	parameter->addAsyncCoalescedListener(this);

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
	nameWindow.addTextEditor("val", parameter->stringValue(), "Value");

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
				parameter->setRange(newMin, newMax);
			}
		}

		float newValue = nameWindow.getTextEditorContents("val").getFloatValue();
		parameter->setValue(newValue);
	}
}

void ParameterUI::paintOverChildren(Graphics & g)
{
	ControllableUI::paintOverChildren(g);
	if (parameter->controlMode == Parameter::EXPRESSION)
	{
		Colour c = LIGHTCONTOUR_COLOR;
		ScriptExpression::ExpressionState s = parameter->expression->state;
		if (s == ScriptExpression::EXPRESSION_ERROR) c = Colours::red.brighter(.3f);
		else if (s == ScriptExpression::EXPRESSION_LOADED) c = Colours::limegreen;

		g.setColour(c.withAlpha(.2f));
		g.fillRoundedRectangle(getLocalBounds().toFloat(),1);
	}
}

void ParameterUI::addPopupMenuItems(PopupMenu * p)
{
	p->addItem(1, "Reset value");

	if (!parameter->lockManualControlMode && parameter->isEditable && !parameter->isControllableFeedbackOnly)
	{
		PopupMenu controlModeMenu;
		controlModeMenu.addItem(10, "Manual");
		controlModeMenu.addItem(11, "Expression");
		//controlModeMenu.addItem(12, "Reference");
		p->addSubMenu("Control Mode", controlModeMenu);
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
	}
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
