/*
  ==============================================================================

	ParameterUI.cpp
	Created: 8 Mar 2016 3:48:44pm
	Author:  bkupe

  ==============================================================================
*/

bool ParameterUI::showAlwaysNotifyOption = true;
bool ParameterUI::showControlModeOption = true;

int ParameterUI::currentFocusOrderIndex = 0;
std::function<void(ParameterUI*)> ParameterUI::customShowEditRangeWindowFunction = nullptr;

ParameterUI::ParameterUI(Parameter* parameter) :
	ControllableUI(parameter),
	parameter(parameter),
	setUndoableValueOnMouseUp(true),
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
	//if (parameter->isControllableFeedbackOnly) return;
	if (parameter == nullptr || parameter.wasObjectDeleted()) return;
	std::unique_ptr<Component> editComponent(getEditValueComponent());
	CallOutBox* box = &CallOutBox::launchAsynchronously(std::move(editComponent), localAreaToGlobal(getLocalBounds()), nullptr);
	box->setArrowSize(8);
}

Component* ParameterUI::getEditValueComponent()
{
	return new ValueEditCalloutComponent(parameter);
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

	nameWindow.addTextEditor("minVal", parameter->hasRange() ? String((float)parameter->minimumValue) : "", "Minimum");
	nameWindow.addTextEditor("maxVal", parameter->hasRange() ? String((float)parameter->maximumValue) : "", "Maximum");

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
			parameter->setRange(newMin, jmax(newMin, newMax));
		}
	}
}

void ParameterUI::paintOverChildren(Graphics& g)
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

void ParameterUI::addPopupMenuItems(PopupMenu* p)
{
	if (parameter.wasObjectDeleted()) return;

	if (isInteractable() || parameter->controlMode != Parameter::MANUAL)
	{
		if (isInteractable()) p->addItem(1, "Reset value");

		if (isInteractable())
		{
			p->addSeparator();
			p->addItem(-3, "Show Edit Window");

			if (parameter->canHaveRange && parameter->isCustomizableByUser)
			{
				p->addItem(-4, "Set Range...");
				if (parameter->hasRange()) p->addItem(-5, "Clear Range");
			}

			if(showAlwaysNotifyOption) p->addItem(-6, "Always Notify changes", true, parameter->alwaysNotify);

			addPopupMenuItemsInternal(p);
		}

		if (showControlModeOption)
		{
			p->addSeparator();
			if (!parameter->lockManualControlMode)
			{
				PopupMenu controlModeMenu;
				controlModeMenu.addItem(10, "Manual", true, parameter->controlMode == Parameter::MANUAL);
				controlModeMenu.addItem(11, "Expression", true, parameter->controlMode == Parameter::EXPRESSION);
				controlModeMenu.addItem(12, "Reference", true, parameter->controlMode == Parameter::REFERENCE);
				if (parameter->canBeAutomated) controlModeMenu.addItem(13, "Animation", true, parameter->controlMode == Parameter::AUTOMATION);
				p->addSubMenu("Control Mode", controlModeMenu);
			}
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

void ParameterUI::mouseDoubleClick(const MouseEvent& e)
{
	if (showEditWindowOnDoubleClick && !e.mods.isAnyModifierKeyDown()) showEditWindow();
}

bool ParameterUI::isInteractable(bool falseIfFeedbackOnly)
{
	if (parameter == nullptr || parameter.wasObjectDeleted()) return false;
	return ControllableUI::isInteractable(falseIfFeedbackOnly) || parameter->controlMode != Parameter::ControlMode::MANUAL;
}

void ParameterUI::setNextFocusOrder(Component* focusComponent)
{
	focusComponent->setExplicitFocusOrder(ParameterUI::currentFocusOrderIndex++);
}


double ParameterUI::textToValue(const String& text)
{
	String error;
	Expression e(text, error);
	if (error.isNotEmpty())
	{
		LOGERROR("Error parsing this expression !");
		return 0;
	}
	return e.evaluate();
}

bool ParameterUI::shouldBailOut() {
	bool bailOut = parameter.wasObjectDeleted() || parameter == nullptr;
	// we want a clean deletion no?
	//jassert(!bailOut);
	return bailOut;

}

// see Parameter::AsyncListener

void ParameterUI::controlModeChanged(Parameter*) 
{
	updateUIParams();
}

void ParameterUI::newMessage(const Parameter::ParameterEvent& e) {
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
		controlModeChanged(e.parameter);
		repaint();
		break;
	}
}

ParameterUI::ValueEditCalloutComponent::ValueEditCalloutComponent(WeakReference<Parameter> p) :
	p(p)
{
	if (p.wasObjectDeleted()) return;

	int numValues = p->isComplex() ? p->value.size() : 1;
	for (int i = 0; i < numValues; ++i)
	{
		Label* label = new Label("ValueLabel" + String(i));
		label->addListener(this);
		label->setText(p->isComplex() ? p->value[i].toString() : p->stringValue(), dontSendNotification);
		label->setEditable(true);
		if (p->isComplex()) label->setColour(label->outlineColourId, BG_COLOR);
		addAndMakeVisible(label);
		labels.add(label);
	}

	setSize(100 * numValues, 20);
}

ParameterUI::ValueEditCalloutComponent::~ValueEditCalloutComponent()
{
}

void ParameterUI::ValueEditCalloutComponent::resized()
{
	if (p.wasObjectDeleted())
	{
		if (CallOutBox* b = dynamic_cast<CallOutBox*>(getParentComponent())) b->dismiss();
		return;
	}

	const int gap = 4;
	int numValues = p->isComplex() ? p->value.size() : 1;
	int labelWidth = (getWidth() - (gap * numValues - 1)) / numValues;
	juce::Rectangle<int> r = getLocalBounds();
	for (int i = 0; i < numValues; ++i)
	{
		if (i > 0) r.removeFromLeft(gap);
		labels[i]->setBounds(r.removeFromLeft(labelWidth));
	}
}

void ParameterUI::ValueEditCalloutComponent::paint(Graphics& g)
{
}

void ParameterUI::ValueEditCalloutComponent::labelTextChanged(Label* l)
{
	if (!p.wasObjectDeleted() && p != nullptr)
	{
		var labelVal;

		var oldVal = p->getValue();

		var newVal;
		int numValues = p->isComplex() ? p->value.size() : 1;
		for (int i = 0; i < numValues; ++i)
		{
			if (p->type == Parameter::STRING) newVal.append(labels[i]->getText());
			else newVal.append(ParameterUI::textToValue(labels[i]->getText().replace(",", ".")));
		}

		p->setUndoableValue(oldVal, p->isComplex() ? newVal : newVal[0]);
	}
	else
	{
		if (CallOutBox* b = dynamic_cast<CallOutBox*>(getParentComponent())) b->dismiss();
	}
}

void ParameterUI::ValueEditCalloutComponent::editorHidden(Label* l, TextEditor&)
{
	CallOutBox* b = dynamic_cast<CallOutBox*>(getParentComponent());
	if (b != nullptr)
	{
		if (l == labels[labels.size() - 1]) b->dismiss();
	}
}

void ParameterUI::ValueEditCalloutComponent::parentHierarchyChanged()
{
	if (labels[0]->isShowing())
	{
		labels[0]->grabKeyboardFocus();
		labels[0]->showEditor();
	}
}