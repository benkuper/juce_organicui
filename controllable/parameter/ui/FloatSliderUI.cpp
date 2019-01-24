#include "FloatSliderUI.h"
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
	bgColor(BG_COLOR.darker(.1f).withAlpha(.8f)),
	useCustomColor(false),
	addToUndoOnMouseUp(true),
	fixedDecimals(3),
	shouldRepaint(true)
{
    assignOnMousePosDirect = false;
    changeParamOnMouseUpOnly = false;
    orientation = HORIZONTAL;
    scaleFactor = 1;

	setSize(100, 16);
}

FloatSliderUI::~FloatSliderUI()
{
}

void FloatSliderUI::setFrontColor(Colour c)
{
	customColor = c;
	useCustomColor = true;
	repaint();
}

void FloatSliderUI::resetFrontColor()
{
	useCustomColor = false;
	repaint();
}


void FloatSliderUI::paint(Graphics & g)
{

	if (shouldBailOut()) return;

	Colour baseColour = useCustomColor ? customColor : (isInteractable()? PARAMETER_FRONT_COLOR : FEEDBACK_COLOR);
    Colour c = (isMouseButtonDown() && changeParamOnMouseUpOnly) ? HIGHLIGHT_COLOR : baseColour;

    juce::Rectangle<int> sliderBounds = getLocalBounds();


    float normalizedValue = getParamNormalizedValue();
    g.setColour(bgColor);
    g.fillRoundedRectangle(sliderBounds.toFloat(), 2);
	
    g.setColour(c);
    float drawPos = 0;
    if (orientation == HORIZONTAL)
    {
        drawPos = changeParamOnMouseUpOnly ? getMouseXYRelative().x : normalizedValue*getWidth();

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
        drawPos = changeParamOnMouseUpOnly ? getMouseXYRelative().y : normalizedValue*getHeight();
        g.fillRoundedRectangle(sliderBounds.removeFromBottom((int)drawPos).reduced(1).toFloat(), 2);
    }


    if(showLabel || showValue){
        //Colour textColor = normalizedValue > .5f?Colours::darkgrey : Colours::grey;

		g.setColour(Colours::grey);

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
			text += parameter->niceName;
			if (showValue) text += " : ";
		}


		if (showValue) text += String::formatted("%." + String(fixedDecimals + 1) + "f", parameter->floatValue()).dropLastCharacters(1);
		g.setFont((float)jmin<int>(getHeight() - 4,16));
        g.drawFittedText(text, destRect, Justification::centred,1);
    }

	g.setColour(bgColor.brighter(.1f));
	g.drawRoundedRectangle(sliderBounds.toFloat(), 2, 1);

}

void FloatSliderUI::mouseDownInternal(const MouseEvent & e)
{
    initValue = getParamNormalizedValue();
    setMouseCursor(MouseCursor::NoCursor);

	if (e.mods.isRightButtonDown()) {
		parameter->resetValue();
	}

    if (e.mods.isLeftButtonDown() && assignOnMousePosDirect)
    {
        setParamNormalizedValue(getValueFromMouse());
    }
    else
    {
        repaint();
    }
}



void FloatSliderUI::mouseDrag(const MouseEvent & e)
{
	if (!isInteractable()) return;

	if(changeParamOnMouseUpOnly) shouldRepaint = true;
    else
    {
		if (e.mods.isLeftButtonDown())
		{
			if (assignOnMousePosDirect)
			{
				setParamNormalizedValue(getValueFromMouse());
			}else
			{
				float diffValue = getValueFromPosition((e.getPosition() - e.getMouseDownPosition()));
				if (orientation == VERTICAL) diffValue -= 1;

				setParamNormalizedValue(initValue + diffValue*scaleFactor);
			}
		}
    }
}

void FloatSliderUI::mouseUpInternal(const MouseEvent &)
{
	BailOutChecker checker (this);
	
	if (changeParamOnMouseUpOnly)
	{
		setParamNormalizedValueUndoable(initValue, getValueFromMouse());
	} else
	{
		setParamNormalizedValueUndoable(initValue, parameter->getNormalizedValue());
	}
	
    repaint();

    if(!checker.shouldBailOut()){
        setMouseCursor(MouseCursor::NormalCursor);
    }
}

float FloatSliderUI::getValueFromMouse()
{
    return getValueFromPosition(getMouseXYRelative());
}

float FloatSliderUI::getValueFromPosition(const Point<int> &pos)
{
    if (orientation == HORIZONTAL) return (pos.x*1.0f / getWidth());
    else return 1-(pos.y*1.0f/ getHeight());
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
    return parameter->getNormalizedValue();
}


void FloatSliderUI::valueChanged(const var &) {
	shouldRepaint = true;
};


void FloatSliderUI::rangeChanged(Parameter *) {
	repaint();
}

void FloatSliderUI::visibilityChanged()
{
	if (isVisible())
	{
		//DBG(parameter->niceName << " start Timer");
#if JUCE_MAC
		startTimerHz(20); //20 fps for slider on mac because of bad UI handling
#else
        startTimerHz(30); //30 fps for slider
#endif
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
