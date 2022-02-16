/*
 ==============================================================================

 FloatSliderUI.cpp
 Created: 8 Mar 2016 3:46:27pm
 Author:  bkupe

 ==============================================================================
 */


 //==============================================================================
FloatSliderUI::FloatSliderUI(Array<Parameter*> parameters) :
	ParameterUI(parameters, PARAMETERUI_DEFAULT_TIMER),
	addToUndoOnMouseUp(true),
	fixedDecimals(3),
	initValue(0),
	initNormalizedValue(0),
	lastDrawPos(0)
{
	assignOnMousePosDirect = false;
	changeParamOnMouseUpOnly = false;
	orientation = HORIZONTAL;

	setWantsKeyboardFocus(true);
	ParameterUI::setNextFocusOrder(this);

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size
}

FloatSliderUI::~FloatSliderUI()
{
}

void FloatSliderUI::paint(Graphics& g)
{

	if (shouldBailOut())
	{
		DBG("BAIL OUT !");
		return;
	}

	if (parameter.wasObjectDeleted()) return;

	juce::Rectangle<int> sliderBounds = getLocalBounds();

	Colour baseColor = useCustomFGColor ? customFGColor : (isInteractable() ? PARAMETER_FRONT_COLOR.darker() : FEEDBACK_COLOR);
	drawBG(g);

	Colour c = (isMouseButtonDown() && changeParamOnMouseUpOnly) ? HIGHLIGHT_COLOR : baseColor;
	g.setColour(c);
	float drawPos = getDrawPos();
	if (orientation == HORIZONTAL)
	{
		if ((float)parameter->minimumValue < 0 && (float)parameter->maximumValue >= 0)
		{
			float zeroPos = jmap<float>(0, parameter->minimumValue, parameter->maximumValue, 0, getWidth());
			g.drawVerticalLine(zeroPos, sliderBounds.getY() + 1, sliderBounds.getBottom() - 1);
			if (parameter->floatValue() != 0) g.fillRoundedRectangle(sliderBounds.withLeft(jmin<float>(drawPos, zeroPos)).withRight(jmax<float>(drawPos, zeroPos + 1)).reduced(0, 1).toFloat(), 2);
		}
		else
		{
			g.fillRoundedRectangle(sliderBounds.removeFromLeft((int)drawPos).reduced(1).toFloat(), 2);
		}
	}
	else if (orientation == VERTICAL)
	{

		g.fillRoundedRectangle(sliderBounds.removeFromBottom((int)drawPos).reduced(1).toFloat(), 2);
	}
	else if (orientation == ROTARY)
	{
		if ((float)parameter->minimumValue < 0 && (float)parameter->maximumValue >= 0)
		{
			if (parameter->floatValue() != 0) drawRotary(g, c, jmin<float>(parameter->value, 0), jmax<float>(parameter->value, 0), 2);
			else drawRotary(g, c, -.01f, .01f, 2);
		}
		else
		{
			drawRotary(g, c, parameter->minimumValue, parameter->value, 2);
		}
	}

	lastDrawPos = drawPos;

	if (showLabel || showValue) {

		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);

		juce::Rectangle<int> destRect;

		if (orientation == VERTICAL) {
			//destRect = juce::Rectangle<int>(0, 0, 100, 100);
			juce::AffineTransform at;
			at = at.rotated((float)(-MathConstants<double>::pi / 2.0f));// , sliderBounds.getCentreX(), sliderBounds.getCentreY());
			at = at.translated(0.f, (float)getHeight());
			g.addTransform(at);
			destRect = juce::Rectangle<int>(0, 0, getHeight(), getWidth()).withSizeKeepingCentre(getHeight(), 10);
		}
		else if (orientation == HORIZONTAL)
		{
			destRect = getLocalBounds();
		}
		else if (orientation == ROTARY)
		{
			float size = jmin(getWidth(), getHeight());
			destRect = getLocalBounds().withSizeKeepingCentre(size / 2, size / 2);
		}

		String text = "";
		if (showLabel)
		{
			text += customLabel.isNotEmpty() ? customLabel : parameter->niceName;
			if (showValue) text += orientation == ROTARY ? "\n" : " : ";
		}


		if (showValue)
		{
			lastValueText = getValueText();
			text += lastValueText;
		}

		float fontSizeLimit = orientation == HORIZONTAL ? destRect.getHeight() : (orientation == VERTICAL ? destRect.getWidth() : destRect.getHeight() / 3);
		g.setFont((float)jmin<int>(fontSizeLimit - 6, 16));

		if (parameter->isOverriden) g.setFont(g.getCurrentFont().boldened());
		g.drawFittedText(text, destRect, Justification::centred, orientation == ROTARY ? 2 : 1);
	}
}

void FloatSliderUI::drawBG(Graphics& g)
{
	Colour bgColor = useCustomBGColor ? customBGColor : BG_COLOR.darker(.1f).withAlpha(.8f);

	if (orientation == HORIZONTAL || orientation == VERTICAL)
	{
		g.setColour(bgColor);
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 2);
	}
	else if (orientation == ROTARY)
	{
		drawRotary(g, bgColor, parameter->minimumValue, parameter->maximumValue);
	}
}

void FloatSliderUI::mouseDownInternal(const MouseEvent& e)
{
	initValue = parameter->floatValue();
	initNormalizedValue = getParamNormalizedValue();

	if (e.mods.isLeftButtonDown())
	{
		if (e.mods.isCommandDown())
		{
			parameter->setValue(parameter->defaultValue, false, true, true);

		}
		else
		{
			setMouseCursor(MouseCursor::NoCursor);
			if (assignOnMousePosDirect)	setParamNormalizedValue(getNormalizedValueFromMouse());
		}
	}

	repaint();
}



void FloatSliderUI::mouseDrag(const MouseEvent& e)
{
	if (!isInteractable()) return;

	if (changeParamOnMouseUpOnly) repaint();
	else
	{
		if (e.mods.isLeftButtonDown() && !e.mods.isCommandDown())
		{
			if (assignOnMousePosDirect)
			{
				setParamNormalizedValue(getNormalizedValueFromMouse());
			}
			else
			{
				float normVal = getNormalizedValueFromMouseDrag(e);
				setParamNormalizedValue(normVal);
			}
		}
	}
}

void FloatSliderUI::mouseUpInternal(const MouseEvent& e)
{
	BailOutChecker checker(this);

	if (e.mods.isLeftButtonDown() && e.mods.isCommandDown()) return;

	if (setUndoableValueOnMouseUp)
	{
		if (changeParamOnMouseUpOnly)
		{
			if (initNormalizedValue != getNormalizedValueFromMouse()) setParamNormalizedValueUndoable(initNormalizedValue, getNormalizedValueFromMouse());
		}
		else
		{
			if (initNormalizedValue != getNormalizedValueFromMouse()) setParamNormalizedValueUndoable(initNormalizedValue, parameter->getNormalizedValue());
		}
	}

	repaint();

	if (!checker.shouldBailOut()) {
		setMouseCursor(MouseCursor::NormalCursor);
	}
}

float FloatSliderUI::getNormalizedValueFromMouse()
{
	return getNormalizedValueFromPosition(getMouseXYRelative());
}

float FloatSliderUI::getNormalizedValueFromMouseDrag(const MouseEvent& e)
{
	float scaleFactor = e.mods.isAltDown() ? .5f : 1;
	Point<int> relPos = e.getOffsetFromDragStart() * scaleFactor;
	float normRelVal = getNormalizedValueFromPosition(relPos);
	if (orientation == VERTICAL) normRelVal = -(1 - normRelVal);
	return initNormalizedValue + normRelVal;
}

float FloatSliderUI::getNormalizedValueFromPosition(const Point<int>& pos)
{
	if (orientation == HORIZONTAL || orientation == ROTARY) return (pos.x * 1.0f / getWidth());
	else return 1 - (pos.y * 1.0f / getHeight());
}

int FloatSliderUI::getDrawPos()
{
	float normalizedValue = getParamNormalizedValue();
	if (orientation == HORIZONTAL) return changeParamOnMouseUpOnly ? getMouseXYRelative().x : normalizedValue * getWidth();
	else return changeParamOnMouseUpOnly ? getMouseXYRelative().y : normalizedValue * getHeight();
}

String FloatSliderUI::getValueText() const
{
	String v = fixedDecimals == -1 ? String(parameter->intValue()) : (fixedDecimals == 0 ? parameter->stringValue() : String::formatted("%." + String(fixedDecimals + 1) + "f", parameter->floatValue()).dropLastCharacters(1));
	return prefix + v + suffix;
}

float FloatSliderUI::getAngleForValue(float val, float angleReduction)
{
	float relVal = jmap<float>(val, parameter->minimumValue, parameter->maximumValue, 0, 1);
	return degreesToRadians(angleReduction + relVal * (270 - angleReduction));
}

void FloatSliderUI::drawRotary(Graphics& g, Colour c, float startPos, float endPos, float reduction)
{
	if (startPos == endPos) return;

	Point<int> centre = getLocalBounds().getCentre();

	float size = jmin<float>(getWidth(), getHeight()) - 4;
	float thickness = size * .2f;

	float offset = degreesToRadians(-180 + 45.0f);

	float sa = getAngleForValue(startPos, reduction);
	float ea = getAngleForValue(endPos, reduction);

	float s2 = size / 2;

	Path p;
	p.addCentredArc(centre.x, centre.y, s2 - reduction, s2 - reduction, offset, sa, ea, true);
	p.addCentredArc(centre.x, centre.y, s2 - thickness + reduction, s2 - thickness + reduction, offset, ea, sa);
	p.closeSubPath();

	g.setColour(c);
	g.fillPath(p.createPathWithRoundedCorners(4));

	g.setColour(c.brighter(.2f));
	g.strokePath(p, PathStrokeType(1));
}

void FloatSliderUI::setParamNormalizedValueUndoable(float oldValue, float newValue)
{
	parameter->setUndoableNormalizedValue(oldValue, newValue);
}

void FloatSliderUI::setParamNormalizedValue(float value)
{
	parameter->setNormalizedValue(value);
}

float FloatSliderUI::getParamNormalizedValue()
{
	return (float)parameter->getNormalizedValue();
}


void FloatSliderUI::valueChanged(const var&) {

	if (parameter.wasObjectDeleted()) return;

	//don't repaint if there is no visible change (useful for very small and fast changes like RMS)
	if (getDrawPos() != lastDrawPos
		|| (showValue && getValueText() != lastValueText))
	{
		shouldRepaint = true;
	}
};


void FloatSliderUI::rangeChanged(Parameter*) {
	shouldRepaint = true;
}


void FloatSliderUI::focusGained(FocusChangeType cause)
{
	ParameterUI::focusGained(cause);
	if (cause == FocusChangeType::focusChangedByTabKey)
	{
		showEditWindow();
	}
}
