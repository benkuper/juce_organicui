/*
 ==============================================================================

 FloatSliderUI.cpp
 Created: 8 Mar 2016 3:46:27pm
 Author:  bkupe

 ==============================================================================
 */


//==============================================================================
FloatSliderUI::FloatSliderUI(Parameter * parameter) :
	ParameterUI(parameter),
	addToUndoOnMouseUp(true),
	fixedDecimals(3),
	initValue(0),
	initNormalizedValue(0),
#if JUCE_MAC
	updateRate(15),
#else
	updateRate(30),
#endif
    shouldRepaint(true),
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

void FloatSliderUI::paint(Graphics & g)
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
			g.drawVerticalLine(zeroPos, sliderBounds.getY()+1, sliderBounds.getBottom()-1);
			if(parameter->floatValue() != 0) g.fillRoundedRectangle(sliderBounds.withLeft(jmin<float>(drawPos, zeroPos)).withRight(jmax<float>(drawPos, zeroPos+1)).reduced(0,1).toFloat(), 2);
		}else
		{
			g.fillRoundedRectangle(sliderBounds.removeFromLeft((int)drawPos).reduced(1).toFloat(), 2);
		}
    }
    else {
		
        g.fillRoundedRectangle(sliderBounds.removeFromBottom((int)drawPos).reduced(1).toFloat(), 2);
    }

	lastDrawPos = drawPos;

    if(showLabel || showValue){

		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);

        sliderBounds = getLocalBounds();
		juce::Rectangle<int> destRect;

        if(orientation == VERTICAL){
            //destRect = juce::Rectangle<int>(0, 0, 100, 100);
            juce::AffineTransform at;
            at = at.rotated((float)(-double_Pi / 2.0f));// , sliderBounds.getCentreX(), sliderBounds.getCentreY());
            at = at.translated(0.f,(float)sliderBounds.getHeight());
            g.addTransform(at);
            destRect = juce::Rectangle<int>(0, 0, sliderBounds.getHeight(), sliderBounds.getWidth()).withSizeKeepingCentre(sliderBounds.getHeight(), 10);
        }else
        {
            destRect = sliderBounds.withSizeKeepingCentre(sliderBounds.getWidth(), getHeight());
        }
		String text = "";
		if (showLabel)
		{
			text += customLabel.isNotEmpty()?customLabel:parameter->niceName;
			if (showValue) text += " : ";
		}


		if (showValue)
		{
			lastValueText = getValueText();
			text += lastValueText;
		}
		
		float fontSizeLimit = orientation == HORIZONTAL ? getHeight() : getWidth();
		g.setFont((float)jmin<int>(fontSizeLimit - 6, 16));
		
		if (parameter->isOverriden) g.setFont(g.getCurrentFont().boldened());
        g.drawFittedText(text, destRect, Justification::centred,1);
    }
}

void FloatSliderUI::drawBG(Graphics &g)
{
	Colour bgColor = useCustomBGColor ? customBGColor : BG_COLOR.darker(.1f).withAlpha(.8f);
	g.setColour(bgColor);
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 2);
}

void FloatSliderUI::mouseDownInternal(const MouseEvent & e)
{
	initValue = parameter->floatValue();
    initNormalizedValue = getParamNormalizedValue();

    if (e.mods.isLeftButtonDown())
    {
		if (e.mods.isCommandDown())
		{
			parameter->setValue(parameter->defaultValue, false, true, true);

		}else 
		{
			setMouseCursor(MouseCursor::NoCursor);
			if (assignOnMousePosDirect)	setParamNormalizedValue(getNormalizedValueFromMouse());
		}
    }
    
	repaint();
}



void FloatSliderUI::mouseDrag(const MouseEvent & e)
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
			}else
			{
				float normVal = getNormalizedValueFromMouseDrag(e);
				setParamNormalizedValue(normVal);
			}
		}
    }
}

void FloatSliderUI::mouseUpInternal(const MouseEvent& e)
{
	BailOutChecker checker (this);
	
	if (e.mods.isLeftButtonDown() && e.mods.isCommandDown()) return;

	if (setUndoableValueOnMouseUp)
	{
		if (changeParamOnMouseUpOnly)
		{
			if (initNormalizedValue !=  getNormalizedValueFromMouse()) setParamNormalizedValueUndoable(initNormalizedValue, getNormalizedValueFromMouse());
		}
		else
		{
			if (initNormalizedValue != getNormalizedValueFromMouse()) setParamNormalizedValueUndoable(initNormalizedValue, parameter->getNormalizedValue());
		}
	}
	
    repaint();

    if(!checker.shouldBailOut()){
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

float FloatSliderUI::getNormalizedValueFromPosition(const Point<int> &pos)
{
	if (orientation == HORIZONTAL) return (pos.x * 1.0f / getWidth());
	else return 1 - (pos.y * 1.0f / getHeight());
}

int FloatSliderUI::getDrawPos()
{
	float normalizedValue = getParamNormalizedValue();
	if(orientation == HORIZONTAL) return changeParamOnMouseUpOnly ? getMouseXYRelative().x : normalizedValue * getWidth();
	else return changeParamOnMouseUpOnly ? getMouseXYRelative().y : normalizedValue * getHeight();
}

String FloatSliderUI::getValueText() const
{
	String v = fixedDecimals == -1 ? String(parameter->intValue()) : (fixedDecimals == 0 ? parameter->stringValue() : String::formatted("%." + String(fixedDecimals + 1) + "f", parameter->floatValue()).dropLastCharacters(1));
	return prefix + v + suffix;
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


void FloatSliderUI::valueChanged(const var &) {

	if (parameter.wasObjectDeleted()) return;

	//don't repaint if there is no visible change (useful for very small and fast changes like RMS)
	if (getDrawPos() != lastDrawPos 
		|| (showValue && getValueText() != lastValueText))
	{
		shouldRepaint = true;
	}
};


void FloatSliderUI::rangeChanged(Parameter *) {
	shouldRepaint = true;
}

void FloatSliderUI::visibilityChanged()
{
	if (isVisible())
	{
        startTimerHz(updateRate); //30 fps for slider
	} else
	{
		//DBG(parameter->niceName << " stop Timer");
		stopTimer();
	}
}

void FloatSliderUI::timerCallback()
{
	if (!shouldRepaint) return;
	shouldRepaint = false;
	repaint();
}

void FloatSliderUI::focusGained(FocusChangeType cause)
{
	ParameterUI::focusGained(cause);
	if (cause == FocusChangeType::focusChangedByTabKey)
	{
		showEditWindow();
	}
}
