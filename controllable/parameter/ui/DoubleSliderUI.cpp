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
	yParam("Y", "yParam", parameter->y,parameter->minimumValue[0],parameter->maximumValue[0]),
	xSlider(&xParam), ySlider(&yParam)
{
	xParam.defaultValue = 0;
	yParam.defaultValue = 0;

	xParam.addAsyncCoalescedListener(this);
	yParam.addAsyncCoalescedListener(this);

	addAndMakeVisible(&xSlider);
	addAndMakeVisible(&ySlider);

	setForceFeedbackOnlyInternal(); //force update

}

DoubleSliderUI::~DoubleSliderUI()
{
	xParam.removeAsyncParameterListener(this);
	yParam.removeAsyncParameterListener(this);

}

void DoubleSliderUI::resized()
{
	Rectangle<int> r = getLocalBounds();
	xSlider.setBounds(r.removeFromLeft(r.getWidth() / 2 - 5));
	ySlider.setBounds(r.removeFromRight(r.getWidth() - 10));
}


void DoubleSliderUI::setForceFeedbackOnlyInternal()
{
	bool val = parameter->isControllableFeedbackOnly || !parameter->isEditable || forceFeedbackOnly;
	xSlider.setForceFeedbackOnly(val);
	ySlider.setForceFeedbackOnly(val);
}

void DoubleSliderUI::newMessage(const Parameter::ParameterEvent & e)
{
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
	} else if (e.type == Parameter::ParameterEvent::BOUNDS_CHANGED)
	{
		xParam.setRange(e.parameter->minimumValue[0], e.parameter->maximumValue[0]);
		yParam.setRange(e.parameter->minimumValue[1], e.parameter->maximumValue[1]);
	}
}
