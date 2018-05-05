#include "DoubleSliderUI.h"
/*
  ==============================================================================

    DoubleSliderUI.cpp
    Created: 25 Oct 2016 11:46:46am
    Author:  bkupe

  ==============================================================================
*/

DoubleSliderUI::DoubleSliderUI(Point2DParameter * parameter) :
	ParameterUI(parameter),
	p2d(parameter),
	xParam("X","xParam",parameter->x, parameter->minimumValue[0],parameter->maximumValue[0]),
	yParam("Y", "yParam", parameter->y, parameter->minimumValue[1],parameter->maximumValue[1]),
	xSlider(&xParam), ySlider(&yParam)
{
	xParam.defaultValue = 0;
	yParam.defaultValue = 0;
	xParam.defaultUI = parameter->defaultUI;
	yParam.defaultUI = parameter->defaultUI;

	xParam.addAsyncParameterListener(this);
	yParam.addAsyncParameterListener(this);

	xSlider.addToUndoOnMouseUp = false;
	ySlider.addToUndoOnMouseUp = false;

	addAndMakeVisible(&xSlider);
	addAndMakeVisible(&ySlider);

	setInterceptsMouseClicks(true, true);

	setForceFeedbackOnlyInternal(); //force update

}

DoubleSliderUI::~DoubleSliderUI()
{
	xParam.removeAsyncParameterListener(this);
	yParam.removeAsyncParameterListener(this);

}

void DoubleSliderUI::mouseDownInternal(const MouseEvent &)
{
	mouseDownValue = parameter->value;
}

void DoubleSliderUI::mouseUpInternal(const MouseEvent &)
{
	parameter->setUndoableValue(mouseDownValue, parameter->value);
}

void DoubleSliderUI::resized()
{
 juce::Rectangle<int> r = getLocalBounds();
	xSlider.setBounds(r.removeFromLeft(r.getWidth() / 2 - 5));
	ySlider.setBounds(r.removeFromRight(r.getWidth() - 10));
}

void DoubleSliderUI::showEditWindow()
{
	AlertWindow nameWindow("Change point 2D params", "Set new values and bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);
	
	const String coordNames[3]{ "X","Y","Z" };
	for(int i=0;i<2;i++) nameWindow.addTextEditor("val"+String(i), p2d->value[i].toString(), "Value "+ coordNames[i]);

	if (parameter->isCustomizableByUser)
	{
		for (int i = 0; i < 2; i++)
		{
			nameWindow.addTextEditor("minVal" + String(i), p2d->minimumValue[i].toString(), "Minimum " + coordNames[i]);
			nameWindow.addTextEditor("maxVal" + String(i), p2d->maximumValue[i].toString(), "Maximum " + coordNames[i]);
		}
	}

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		if (parameter->isCustomizableByUser)
		{
			float newMins[2];
			float newMaxs[2];
			for (int i = 0; i < 2; i++)
			{
				newMins[i] = nameWindow.getTextEditorContents("minVal"+String(i)).getFloatValue();
				newMaxs[i] = nameWindow.getTextEditorContents("maxVal"+String(i)).getFloatValue();
			}
			p2d->setBounds(newMins[0], newMins[1], newMaxs[0], newMaxs[1]);

		}

		float newVals[2];
		for(int i=0;i<2;i++) newVals[i] = nameWindow.getTextEditorContents("val"+String(i)).getFloatValue();
		p2d->setPoint(newVals[0],newVals[1]);
	}
}

void DoubleSliderUI::rangeChanged(Parameter * p)
{
	if (p != parameter) return;
	xParam.setRange(parameter->minimumValue[0], parameter->maximumValue[0]);
	yParam.setRange(parameter->minimumValue[1], parameter->maximumValue[1]);
}


void DoubleSliderUI::setForceFeedbackOnlyInternal()
{
	bool val = parameter->isControllableFeedbackOnly || !parameter->isEditable || forceFeedbackOnly;
	xSlider.setForceFeedbackOnly(val);
	ySlider.setForceFeedbackOnly(val);
}

void DoubleSliderUI::newMessage(const Parameter::ParameterEvent & e)
{
	ParameterUI::newMessage(e);
	if (e.parameter == parameter)
	{
		xParam.setValue(((Point2DParameter *)e.parameter)->x); 
		yParam.setValue(((Point2DParameter *)e.parameter)->y);

	} else if (e.parameter == &xParam)
	{
		if(xParam.floatValue() != p2d->x) p2d->setPoint(xParam.floatValue(), yParam.floatValue());
	} else if (e.parameter == &yParam)
	{
		if (yParam.floatValue() != p2d->y) p2d->setPoint(xParam.floatValue(), yParam.floatValue());
	}
}
