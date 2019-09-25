#include "ParameterUI.h"
/*
  ==============================================================================

	ParameterUI.cpp
	Created: 8 Mar 2016 3:48:44pm
	Author:  bkupe

  ==============================================================================
*/


int ParameterUI::currentFocusOrderIndex = 0;
std::function<void(ParameterUI*)> ParameterUI::customShowEditRangeWindowFunction = nullptr;

ParameterUI::ParameterUI(Parameter * parameter) :
	ControllableUI(parameter),
	parameter(parameter),
	showEditWindowOnDoubleClick(true),
	showValue(true),
	useCustomBGColor(false),
	useCustomFGColor(false)
{
	parameter->addAsyncCoalescedParameterListener(this);

	//setSize(100, 16);
}

ParameterUI::~ParameterUI()
{
	if (!parameter.wasObjectDeleted() && parameter != nullptr) {
		parameter->removeAsyncParameterListener(this);
		parameter = nullptr;
	}
}

void ParameterUI::showEditWindowInternal()
{
	if (parameter->isControllableFeedbackOnly) return;

	AlertWindow nameWindow("Set a value", "Set a new value for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	nameWindow.addTextEditor("val", parameter->stringValue(), "Value");

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		String valString = nameWindow.getTextEditorContents("val");
		if (parameter->type == Parameter::STRING) parameter->setUndoableValue(parameter->getValue(), valString);
		else parameter->setUndoableValue(parameter->getValue(), valString.replace(",", ".").getFloatValue());
	}
}

void ParameterUI::showEditRangeWindow()
{
	if (customShowEditRangeWindowFunction != nullptr) customShowEditRangeWindowFunction(this);
	else showEditRangeWindowInternal();
}

void ParameterUI::showEditRangeWindowInternal()
{
	if (!parameter->canHaveRange || !parameter->isCustomizableByUser) return;

	AlertWindow nameWindow("Set the range", "Set a new range for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	nameWindow.addTextEditor("minVal", parameter->hasRange()?String((float)parameter->minimumValue):"", "Minimum");
	nameWindow.addTextEditor("maxVal", parameter->hasRange()?String((float)parameter->maximumValue):"", "Maximum");

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		if (parameter->type == Parameter::FLOAT || parameter->type == Parameter::INT)
		{
			String minRangeString = nameWindow.getTextEditorContents("minVal");
			String maxRangeString = nameWindow.getTextEditorContents("maxVal");
			float newMin = minRangeString.isNotEmpty() ? minRangeString.getFloatValue() : INT32_MIN;
			float newMax = maxRangeString.isNotEmpty() ? maxRangeString.getFloatValue() : INT32_MAX;
			parameter->setRange(newMin, jmax(newMin,newMax));
		}	
	}
}

void ParameterUI::paintOverChildren(Graphics & g)
{
	ControllableUI::paintOverChildren(g);
	if (parameter == nullptr) return;

	switch (parameter->controlMode)
	{
        case Parameter::MANUAL:
            break;
            
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
	if (isInteractable() || parameter->controlMode != Parameter::MANUAL)
	{
		p->addItem(1, "Reset value");

		if (isInteractable())
		{
			p->addSeparator();
			p->addItem(-3, "Show Edit Window");

			if (parameter->canHaveRange && parameter->isCustomizableByUser)
			{
				p->addItem(-4, "Set Range...");
				if (parameter->hasRange()) p->addItem(-5, "Clear Range");
			}

			p->addItem(-6, "Always Notify changes", true, parameter->alwaysNotify);

			addPopupMenuItemsInternal(p);
		}

		p->addSeparator();
		if (!parameter->lockManualControlMode)
		{
			PopupMenu controlModeMenu;
			controlModeMenu.addItem(10, "Manual", true, parameter->controlMode == Parameter::MANUAL);
			controlModeMenu.addItem(11, "Expression", true, parameter->controlMode == Parameter::EXPRESSION);
			controlModeMenu.addItem(12, "Reference", true, parameter->controlMode == Parameter::REFERENCE);
			if(parameter->canBeAutomated) controlModeMenu.addItem(13, "Animation", true, parameter->controlMode == Parameter::AUTOMATION);
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
	case 13: 
	{
		parameter->setControlMode(Parameter::AUTOMATION);
		parameter->automation->setManualMode(false); //created from user menu, not manual
	}
	break;

	case -4: showEditRangeWindow(); break;
	case -5: parameter->clearRange(); break;
	case -6: parameter->alwaysNotify = !parameter->alwaysNotify; break;
	}
}

void ParameterUI::mouseDoubleClick(const MouseEvent & e)
{
	if (showEditWindowOnDoubleClick) showEditWindow();
}

bool ParameterUI::isInteractable()
{
	return ControllableUI::isInteractable() && parameter->controlMode == Parameter::ControlMode::MANUAL;
}

void ParameterUI::setNextFocusOrder(Component * focusComponent)
{
	focusComponent->setExplicitFocusOrder(ParameterUI::currentFocusOrderIndex++);
}


bool ParameterUI::shouldBailOut() {
	bool bailOut = parameter.get() == nullptr;
	// we want a clean deletion no?
	jassert(!bailOut);
	return bailOut;

}

// see Parameter::AsyncListener

void ParameterUI::newMessage(const Parameter::ParameterEvent &e) {
	switch (e.type)
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
