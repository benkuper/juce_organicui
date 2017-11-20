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
	xParam.addAsyncCoalescedListener(this);
	yParam.addAsyncCoalescedListener(this);
	zParam.addAsyncCoalescedListener(this);

	addAndMakeVisible(&xSlider);
	addAndMakeVisible(&ySlider);
	addAndMakeVisible(&zSlider);

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

void TripleSliderUI::newMessage(const Parameter::ParameterEvent & e)
{
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
	} else if (e.type == Parameter::ParameterEvent::BOUNDS_CHANGED)
	{
		xParam.setRange(e.parameter->minimumValue[0], e.parameter->maximumValue[0]);
		yParam.setRange(e.parameter->minimumValue[1], e.parameter->maximumValue[1]);
		zParam.setRange(e.parameter->minimumValue[2], e.parameter->maximumValue[2]);
	}
}
