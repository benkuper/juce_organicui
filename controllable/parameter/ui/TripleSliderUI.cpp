#include "TripleSliderUI.h"
/*
  ==============================================================================

    TripleSliderUI.cpp
    Created: 2 Nov 2016 4:17:34pm
    Author:  bkupe

  ==============================================================================
*/

TripleSliderUI::TripleSliderUI(Point3DParameter * parameter) :
	ParameterUI(parameter),
	p3d(parameter),
	xParam("X", "xParam", parameter->x, parameter->minimumValue[0], parameter->maximumValue[0]),
	yParam("Y", "yParam", parameter->y, parameter->minimumValue[1], parameter->maximumValue[1]),
	zParam("Z", "zParam", parameter->z, parameter->minimumValue[2], parameter->maximumValue[2]),
	xSlider(&xParam), ySlider(&yParam), zSlider(&zParam)
{

	xParam.defaultValue = 0;
	yParam.defaultValue = 0;
	zParam.defaultValue = 0;
	xParam.addAsyncCoalescedParameterListener(this);
	yParam.addAsyncCoalescedParameterListener(this);
	zParam.addAsyncCoalescedParameterListener(this);

	xSlider.addToUndoOnMouseUp = false;
	ySlider.addToUndoOnMouseUp = false;
	zSlider.addToUndoOnMouseUp = false;

	addAndMakeVisible(&xSlider);
	addAndMakeVisible(&ySlider);
	addAndMakeVisible(&zSlider);

	setInterceptsMouseClicks(true, true);

	setForceFeedbackOnlyInternal(); //force update
}

TripleSliderUI::~TripleSliderUI()
{
	xParam.removeAsyncParameterListener(this);
	yParam.removeAsyncParameterListener(this);
	zParam.removeAsyncParameterListener(this);

}

void TripleSliderUI::setForceFeedbackOnlyInternal()
{
	bool val = parameter->isControllableFeedbackOnly || !parameter->isEditable || forceFeedbackOnly;
	xSlider.setForceFeedbackOnly(val);
	ySlider.setForceFeedbackOnly(val);
	zSlider.setForceFeedbackOnly(val);

}

void TripleSliderUI::resized()
{
	Rectangle<int> r = getLocalBounds();
	xSlider.setBounds(r.removeFromLeft(r.getWidth() / 3 - 2));
	ySlider.setBounds(r.removeFromLeft(r.getWidth() /2 - 4));
	zSlider.setBounds(r.removeFromRight(r.getWidth() - 4));

}

void TripleSliderUI::mouseDownInternal(const MouseEvent &)
{
	mouseDownValue = parameter->value;
}

void TripleSliderUI::mouseUpInternal(const MouseEvent &)
{
	parameter->setUndoableValue(mouseDownValue, parameter->value);
}

void TripleSliderUI::showEditWindow()
{
	AlertWindow nameWindow("Change point 2D params", "Set new values and bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	for (int i = 0; i<3; i++) nameWindow.addTextEditor("val" + String(i), p3d->value[i].toString(), "Value " + String(i));

	if (parameter->isCustomizableByUser)
	{
		for (int i = 0; i < 3; i++)
		{
			nameWindow.addTextEditor("minVal" + String(i), p3d->minimumValue[i].toString(), "Minimum " + String(i));
			nameWindow.addTextEditor("maxVal" + String(i), p3d->maximumValue[i].toString(), "Maximum" + String(i));
		}
	}

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		if (parameter->isCustomizableByUser)
		{
			float newMins[3];
			float newMaxs[3];
			for (int i = 0; i < 3; i++)
			{
				newMins[i] = nameWindow.getTextEditorContents("minVal" + String(i)).getFloatValue();
				newMaxs[i] = nameWindow.getTextEditorContents("maxVal" + String(i)).getFloatValue();
			}
			p3d->setBounds(newMins[0], newMins[1], newMins[2], newMaxs[0], newMaxs[1], newMaxs[2]);

		}

		float newVals[3];
		for (int i = 0; i<3; i++) newVals[i] = nameWindow.getTextEditorContents("val" + String(i)).getFloatValue();
		p3d->setVector(newVals[0], newVals[1],newVals[2]);
	}
}

void TripleSliderUI::rangeChanged(Parameter * p)
{
	if (p != parameter) return;
	xParam.setRange(parameter->minimumValue[0], parameter->maximumValue[0]);
	yParam.setRange(parameter->minimumValue[1], parameter->maximumValue[1]);
	zParam.setRange(parameter->minimumValue[2], parameter->maximumValue[2]);
}

void TripleSliderUI::newMessage(const Parameter::ParameterEvent & e)
{
	ParameterUI::newMessage(e);
	if (e.parameter == parameter)
	{
		xParam.setValue(((Point3DParameter *)e.parameter)->x);
		yParam.setValue(((Point3DParameter *)e.parameter)->y);
		zParam.setValue(((Point3DParameter *)e.parameter)->z);

	} else if (e.parameter == &xParam)
	{
		if (xParam.floatValue() != p3d->x) p3d->setVector(xParam.floatValue(), yParam.floatValue(),zParam.floatValue());
	} else if (e.parameter == &yParam)
	{
		if (yParam.floatValue() != p3d->y) p3d->setVector(xParam.floatValue(), yParam.floatValue(), zParam.floatValue());
	}else if (e.parameter == &zParam)
	{
		if (zParam.floatValue() != p3d->z) p3d->setVector(xParam.floatValue(), yParam.floatValue(), zParam.floatValue());
	} 
}
