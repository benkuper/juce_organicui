/*
  ==============================================================================

	ParameterUI.cpp
	Created: 8 Mar 2016 3:48:44pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

juce_ImplementSingleton(ParameterUITimers)


bool ParameterUI::showAlwaysNotifyOption = true;
bool ParameterUI::showControlModeOption = true;

int ParameterUI::currentFocusOrderIndex = 0;
std::function<void(ParameterUI*)> ParameterUI::customShowEditRangeWindowFunction = nullptr;

ParameterUI::ParameterUI(Array<Parameter*> parameters, int paintTimerID) :
	ControllableUI(Inspectable::getArrayAs<Parameter, Controllable>(parameters)),
	parameters(Inspectable::getWeakArray(parameters)),
	parameter(parameters[0]),
	paintTimerID(paintTimerID),
	shouldRepaint(false),
	setUndoableValueOnMouseUp(true),
	showEditWindowOnDoubleClick(true),
	showValue(true),
	useCustomBGColor(false),
	useCustomFGColor(false)
{
	parameter->addAsyncCoalescedParameterListener(this);

	if (paintTimerID != -1) ParameterUITimers::getInstance()->registerParameter(paintTimerID, this);

	//setSize(100, 16);
}

ParameterUI::~ParameterUI()
{
	if (paintTimerID != -1 && ParameterUITimers::getInstanceWithoutCreating() != nullptr)
	{
		ParameterUITimers::getInstance()->unregisterParameter(paintTimerID, this);
	}

	if (!parameter.wasObjectDeleted() && parameter != nullptr) {
		parameter->removeAsyncParameterListener(this);
		parameter = nullptr;
	}

	masterReference.clear();
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
	if (!isInteractable()) return;
	if (customShowEditRangeWindowFunction != nullptr) customShowEditRangeWindowFunction(this);
	else showEditRangeWindowInternal();
}

void ParameterUI::showEditRangeWindowInternal()
{
	if (!parameter->canHaveRange || !parameter->isCustomizableByUser) return;

	AlertWindow* nameWindow = new AlertWindow("Set the range", "Set a new range for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	nameWindow->addTextEditor("minVal", parameter->hasRange() ? String((float)parameter->minimumValue) : "", "Minimum");
	nameWindow->addTextEditor("maxVal", parameter->hasRange() ? String((float)parameter->maximumValue) : "", "Maximum");

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	nameWindow->enterModalState(true, ModalCallbackFunction::create([this, nameWindow](int result)
		{
			if (result)
			{
				if (parameter->type == Parameter::FLOAT || parameter->type == Parameter::INT)
				{
					String minRangeString = nameWindow->getTextEditorContents("minVal");
					String maxRangeString = nameWindow->getTextEditorContents("maxVal");
					float newMin = minRangeString.isNotEmpty() ? minRangeString.getFloatValue() : INT32_MIN;
					float newMax = maxRangeString.isNotEmpty() ? maxRangeString.getFloatValue() : INT32_MAX;
					parameter->setRange(newMin, jmax(newMin, newMax));
				}
			}
		}),
		true
			);
}

void ParameterUI::paintOverChildren(Graphics& g)
{
	ControllableUI::paintOverChildren(g);
	if (parameter == nullptr) return;

	if (parameter->isControlledByParrot)
	{
		Colour c = Colours::rebeccapurple.brighter(.2f);
		g.setColour(c.withAlpha(.2f));
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 1);
		return;
	}

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

void ParameterUI::handlePaintTimer()
{
	if (!shouldRepaint || parameter.wasObjectDeleted() || !isShowing()) return;
	handlePaintTimerInternal();
	shouldRepaint = false;
}

void ParameterUI::handlePaintTimerInternal()
{
	repaint();
}

void ParameterUI::addPopupMenuItems(PopupMenu* p)
{
	if (parameter.wasObjectDeleted()) return;

	if (isInteractable())
	{
		p->addItem(1, "Reset value");
		p->addSeparator();
		p->addItem(-3, "Show Edit Window");
		p->addItem(-20, "Copy Value");
		p->addItem(-21, "Paste Value");

		if (parameter->canHaveRange && parameter->isCustomizableByUser)
		{
			PopupMenu rangeMenu;
			rangeMenu.addItem(-4, "Custom");
			rangeMenu.addSeparator();
			if (parameter->type == Parameter::FLOAT)
			{
				rangeMenu.addItem(-50, "0 : 1");
				rangeMenu.addItem(-51, "-1 : 1");
				rangeMenu.addItem(-52, "-90 : 90");
				rangeMenu.addItem(-53, "0 : 180");
				rangeMenu.addItem(-54, "-180 : 180");
				rangeMenu.addItem(-55, "0 : 360");

			}
			else if (parameter->type == Parameter::INT)
			{
				rangeMenu.addItem(-60, "0 : 100");
				rangeMenu.addItem(-61, "0 : 127");
				rangeMenu.addItem(-62, "0 : 255");
				rangeMenu.addItem(-63, "0 : 65535");
				rangeMenu.addItem(-52, "-90 : 90");
				rangeMenu.addItem(-53, "0 : 180");
				rangeMenu.addItem(-54, "-180 : 180");
				rangeMenu.addItem(-55, "0 : 360");
			}
			else if (parameter->type == Parameter::POINT2D || parameter->type == Parameter::POINT3D)
			{
				rangeMenu.addItem(-70, "0 : 1");
				rangeMenu.addItem(-71, "-1 : 1");
				rangeMenu.addItem(-72, "0 : 100");
				rangeMenu.addItem(-73, "-100 : 100");
			}

			if (parameter->type == Parameter::FLOAT || parameter->type == Parameter::INT)
			{
				int numCustomRanges = ProjectSettings::getInstance()->customRangesCC.controllables.size();
				if (numCustomRanges > 0)
				{
					rangeMenu.addSeparator();
					for (int i = 0; i < numCustomRanges; i++)
					{
						Point2DParameter* rp = dynamic_cast<Point2DParameter*>(ProjectSettings::getInstance()->customRangesCC.controllables[i]);
						rangeMenu.addItem(-100 - i, String(rp->x) + " : " + String(rp->y));
					}
				}
			}

			p->addSubMenu("Set Range...", rangeMenu);
			if (parameter->hasRange()) p->addItem(-5, "Clear Range");
		}
	}

	if (showAlwaysNotifyOption) p->addItem(-6, "Always Notify changes", true, parameter->alwaysNotify);
	addPopupMenuItemsInternal(p);

	if (isInteractable() && showControlModeOption)
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
	case -20: SystemClipboard::copyTextToClipboard(parameter->stringValue()); break;
	case -21:
	{
		var v = SystemClipboard::getTextFromClipboard();
		if (parameter->isComplex())
		{
			String vStr = v;
			if (vStr.startsWithChar('[') && vStr.endsWithChar(']'))
			{
				StringArray vals;
				vals.addTokens(vStr.substring(1, vStr.length() - 2), ",", "");

				if (vals.size() >= parameter->value.size())
				{
					v = var();
					for (int i = 0; i < parameter->value.size(); i++) v.append(vals[i].getFloatValue());
				}
			}
		}

		parameter->setValue(v); break;
	}
	case -50: parameter->setRange(0, 1); break;
	case -51: parameter->setRange(-1, 1); break;
	case -52: parameter->setRange(-90, 90); break;
	case -53: parameter->setRange(0, 180); break;
	case -54: parameter->setRange(-180, 180); break;
	case -55: parameter->setRange(0, 360); break;
	case -60: parameter->setRange(0, 100); break;
	case -61: parameter->setRange(0, 127); break;
	case -62: parameter->setRange(0, 255); break;
	case -63: parameter->setRange(0, 65535); break;

	case -70:
		if (parameter->type == Parameter::POINT2D) ((Point2DParameter*)parameter.get())->setBounds(0, 0, 1, 1);
		else ((Point3DParameter*)parameter.get())->setBounds(0, 0, 0, 1, 1, 1);
		break;
	case -71:
		if (parameter->type == Parameter::POINT2D) ((Point2DParameter*)parameter.get())->setBounds(-1, -1, 1, 1);
		else ((Point3DParameter*)parameter.get())->setBounds(-1, -1, -1, 1, 1, 1);
		break;
	case -72:
		if (parameter->type == Parameter::POINT2D) ((Point2DParameter*)parameter.get())->setBounds(0, 0, 100, 100);
		else ((Point3DParameter*)parameter.get())->setBounds(0, 0, 0, 100, 100, 100);
		break;
	case -73:
		if (parameter->type == Parameter::POINT2D) ((Point2DParameter*)parameter.get())->setBounds(-100, -100, 100, 100);
		else ((Point3DParameter*)parameter.get())->setBounds(-100, -100, -100, 100, 100, 100);
		break;

	default:
	{
		if (id <= -100)
		{
			int numCustomRanges = ProjectSettings::getInstance()->customRangesCC.controllables.size();
			int cid = -(id + 100);
			if (cid < numCustomRanges)
			{
				Point2DParameter* rp = dynamic_cast<Point2DParameter*>(ProjectSettings::getInstance()->customRangesCC.controllables[cid]);
				parameter->setRange(rp->x, rp->y);
			}
		}
		break;
	}
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

void ParameterUI::visibilityChanged()
{
	if (paintTimerID == -1 || ParameterUITimers::getInstanceWithoutCreating() == nullptr) return;

	if (isVisible()) ParameterUITimers::getInstance()->registerParameter(paintTimerID, this);
	else ParameterUITimers::getInstance()->unregisterParameter(paintTimerID, this);
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
	default:
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

	setSize(numValues == 1 ? 200 : 120 * numValues, 20);
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
			if (p->type == Parameter::STRING || p->type == Parameter::TARGET) newVal.append(labels[i]->getText());
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
#if JUCE_MAC
		if (getCurrentlyModalComponent() == nullptr) labels[0]->showEditor();
#else
		labels[0]->showEditor();
#endif
	}
}

ParameterUITimers::ParameterUITimers()
{
#if JUCE_MAC
	startTimer(PARAMETERUI_DEFAULT_TIMER, 1000 / 20); //20 fps drawing on mac
	startTimer(PARAMETERUI_SLOW_TIMER, 1000 / 15); //15 fps drawing on mac
#else
	startTimer(PARAMETERUI_DEFAULT_TIMER, 1000 / 30); //30 fps drawing
	startTimer(PARAMETERUI_SLOW_TIMER, 1000 / 20); //20 fps drawing
#endif

	paramsTimerMap.set(PARAMETERUI_DEFAULT_TIMER, {});
	paramsTimerMap.set(PARAMETERUI_SLOW_TIMER, {});

}

void ParameterUITimers::registerParameter(int timerID, ParameterUI* ui)
{
	if (paramsTimerMap.contains(timerID)) paramsTimerMap.getReference(timerID).addIfNotAlreadyThere(ui);
}

void ParameterUITimers::unregisterParameter(int timerID, ParameterUI* ui)
{
	if (paramsTimerMap.contains(timerID)) paramsTimerMap.getReference(timerID).removeAllInstancesOf(ui);
}

void ParameterUITimers::timerCallback(int timerID)
{
	if (paramsTimerMap.contains(timerID))
	{
		Array<WeakReference<ParameterUI>> params = paramsTimerMap[timerID];

		for (auto& p : params)
		{
			if (p.wasObjectDeleted())
			{
				jassertfalse;
				continue;
			}

			p->handlePaintTimer();
		}
	}
}
