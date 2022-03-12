/*
  ==============================================================================

	FloatParameterLabelUI.cpp
	Created: 10 Dec 2016 10:51:19am
	Author:  Ben

  ==============================================================================
*/

FloatParameterLabelUI::FloatParameterLabelUI(Array<Parameter*> parameters) :
	ParameterUI(parameters, PARAMETERUI_SLOW_TIMER),
	valueLabel(parameters[0]->niceName + "_ValueLabel"),
	maxFontHeight(GlobalSettings::getInstance()->fontSize->floatValue()),
	autoSize(false)
{
	opaqueBackground = true;

	updateUIParams();

	addAndMakeVisible(&valueLabel);
	valueLabel.addListener(this);

	showEditWindowOnDoubleClick = false;

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size

	valueChanged(parameter->getValue());
	feedbackStateChanged();

	ParameterUI::setNextFocusOrder(&valueLabel);

	addMouseListener(this, true);

	handlePaintTimer(); //force update once
}

void FloatParameterLabelUI::setAutoSize(bool value)
{
	autoSize = value;
	valueChanged(parameter->getValue());
}

void FloatParameterLabelUI::setPrefix(const String& _prefix)
{
	if (prefix == _prefix) return;
	prefix = _prefix;
	valueChanged(parameter->stringValue());
}

void FloatParameterLabelUI::setSuffix(const String& _suffix)
{
	if (suffix == _suffix) return;
	suffix = _suffix;
	valueChanged(parameter->stringValue());
}

void FloatParameterLabelUI::updateTooltip()
{
	ParameterUI::updateTooltip();
	valueLabel.setTooltip(tooltip);
}

/*
void FloatParameterLabelUI::paint(Graphics & g)
{
g.fillAll(Colours::purple.withAlpha(.2f));
}
*/

void FloatParameterLabelUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	float fontHeight = jmin<float>((float)r.getHeight(), maxFontHeight);
	if (showLabel)
	{
		nameLabel.setFont(valueLabel.getFont().withHeight(fontHeight));
		nameLabel.setBounds(r.removeFromLeft(nameLabel.getFont().getStringWidth(nameLabel.getText())));
		r.removeFromLeft(4);
	}

	valueLabel.setFont(valueLabel.getFont().withHeight(fontHeight));
	valueLabel.setBounds(r);
}


void FloatParameterLabelUI::mouseDownInternal(const MouseEvent& e)
{
	if (e.mods.isLeftButtonDown() && e.mods.isCommandDown())
	{
		parameter->resetValue();
	}

	valueAtMouseDown = parameter->floatValue();
	valueOffsetSinceMouseDown = 0;
	lastMouseX = e.getMouseDownX();
}


void FloatParameterLabelUI::mouseDrag(const MouseEvent& e)
{
	if (!isInteractable()) return;
	if (valueLabel.isBeingEdited()) return;

	if (valueLabel.getMouseCursor() != MouseCursor::LeftRightResizeCursor)
	{
		valueLabel.setMouseCursor(MouseCursor::LeftRightResizeCursor);
		valueLabel.updateMouseCursor();
	}

	float sensitivity = e.mods.isShiftDown() ? 10 : (e.mods.isAltDown() ? .1f : 1);

	valueOffsetSinceMouseDown += (e.getPosition().x - lastMouseX) * sensitivity / pixelsPerUnit;
	lastMouseX = e.getPosition().x;

	parameter->setValue(valueAtMouseDown + valueOffsetSinceMouseDown);
}

void FloatParameterLabelUI::mouseUpInternal(const MouseEvent& e)
{
	if (!isInteractable()) return;
	if (valueLabel.isBeingEdited()) return;

	valueLabel.setMouseCursor(MouseCursor::NormalCursor);
	valueLabel.updateMouseCursor();

	if (setUndoableValueOnMouseUp)
	{
		if (valueAtMouseDown != parameter->floatValue()) parameter->setUndoableValue(valueAtMouseDown, parameter->floatValue());
	}
}

void FloatParameterLabelUI::updateUIParamsInternal()
{
	valueLabel.setEditable(false, isInteractable());
	valueLabel.setEnabled(isInteractable());

	valueLabel.setJustificationType(Justification::centred);
	valueLabel.setColour(Label::ColourIds::textColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));

	valueLabel.setColour(valueLabel.backgroundColourId, opaqueBackground ? (useCustomBGColor ? customBGColor : BG_COLOR.darker(.3f)) : Colours::transparentBlack);
	valueLabel.setColour(valueLabel.backgroundWhenEditingColourId, Colours::black);
	valueLabel.setColour(CaretComponent::caretColourId, Colours::orange);
	valueLabel.setColour(valueLabel.textWhenEditingColourId, Colours::orange);
	valueLabel.setTooltip(tooltip);

	if (customTextSize > 0) valueLabel.setFont(customTextSize);
	else valueLabel.setFont(Font());

	repaint();
}


String FloatParameterLabelUI::getValueString(const var& val) const
{
	return val.isDouble() ? String(parameter->floatValue(), 3) : val.toString();
}

void FloatParameterLabelUI::valueChanged(const var& v)
{
	shouldRepaint = true;
}

void FloatParameterLabelUI::labelTextChanged(Label*)
{
	parameter->setValue(ParameterUI::textToValue(valueLabel.getText().replace(",", ".")));
}


void FloatParameterLabelUI::handlePaintTimerInternal()
{
	if (parameter.wasObjectDeleted()) return;

	int newStyle = parameter->isOverriden ? Font::bold : Font::plain;
	if (valueLabel.getFont().getStyleFlags() != newStyle) valueLabel.setFont(valueLabel.getFont().withStyle(newStyle));

	valueLabel.setText(prefix + getValueString(parameter->value) + suffix, NotificationType::dontSendNotification);
	if (autoSize)
	{
		int valueLabelWidth = valueLabel.getFont().getStringWidth(valueLabel.getText());
		int tw = valueLabelWidth;
		setSize(tw + 10, (int)valueLabel.getFont().getHeight());
	}
}

//TIME LABEL
TimeLabel::TimeLabel(Array<Parameter*> parameters) :
	FloatParameterLabelUI(parameters),
	showStepsMode(false)
{
	valueChanged(parameter->getValue());
	handlePaintTimer(); //force update once
}

TimeLabel::~TimeLabel()
{
}

void TimeLabel::setShowStepsMode(bool stepsMode)
{
	showStepsMode = stepsMode;
	shouldRepaint = true;
}

void TimeLabel::valueChanged(const var& v)
{
	String timeString = showStepsMode ? String((float)v * ((FloatParameter*)parameter.get())->unitSteps) : StringUtil::valueToTimeString(v);
	FloatParameterLabelUI::valueChanged(timeString);

}

void TimeLabel::labelTextChanged(Label*)
{
	parameter->setValue(showStepsMode ? valueLabel.getText().getFloatValue() / ((FloatParameter*)parameter.get())->unitSteps : StringUtil::timeStringToValue(valueLabel.getText()));
	shouldRepaint = true;
}

String TimeLabel::getValueString(const var& val) const
{
	return showStepsMode ? String((float)val * ((FloatParameter*)parameter.get())->unitSteps) : StringUtil::valueToTimeString(val);
}
