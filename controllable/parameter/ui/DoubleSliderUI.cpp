/*
  ==============================================================================

	DoubleSliderUI.cpp
	Created: 25 Oct 2016 11:46:46am
	Author:  bkupe

  ==============================================================================
*/

DoubleSliderUI::DoubleSliderUI(Point2DParameter* parameter) :
	ParameterUI(parameter),
	p2d(parameter),
	xParam("X", "xParam", parameter->x, parameter->minimumValue[0], parameter->maximumValue[0]),
	yParam("Y", "yParam", parameter->y, parameter->minimumValue[1], parameter->maximumValue[1])
{

	xParam.canHaveRange = parameter->canHaveRange;
	yParam.canHaveRange = parameter->canHaveRange;
	xParam.isCustomizableByUser = parameter->isCustomizableByUser;
	yParam.isCustomizableByUser = parameter->isCustomizableByUser;
	xParam.defaultValue = 0;
	yParam.defaultValue = 0;
	xParam.defaultUI = parameter->defaultUI;
	yParam.defaultUI = parameter->defaultUI;

	xParam.addAsyncParameterListener(this);
	yParam.addAsyncParameterListener(this);

	xSlider.reset((ParameterUI*)xParam.createDefaultUI());
	ySlider.reset((ParameterUI*)yParam.createDefaultUI());

	addAndMakeVisible(xSlider.get());
	addAndMakeVisible(ySlider.get());

	setInterceptsMouseClicks(true, true);

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size

	feedbackStateChanged(); //force update
}

DoubleSliderUI::~DoubleSliderUI()
{
	xParam.removeAsyncParameterListener(this);
	yParam.removeAsyncParameterListener(this);

}

void DoubleSliderUI::mouseDownInternal(const MouseEvent&)
{
	mouseDownValue = parameter->getValue();
}

void DoubleSliderUI::mouseUpInternal(const MouseEvent&)
{
	parameter->setUndoableValue(mouseDownValue, parameter->getValue());
}

void DoubleSliderUI::paint(Graphics& g)
{
	ParameterUI::paint(g);

	if (showLabel)
	{
		Rectangle<int> r = getLocalBounds();
		g.setFont(jlimit(12, 40, jmin(r.getHeight(), r.getWidth()) - 16));
		r = r.removeFromLeft(jmin(g.getCurrentFont().getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 60));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, r, Justification::centred, 1);
	}
}

void DoubleSliderUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	if (showLabel)
	{
		Font font(jlimit(12, 40, jmin(r.getHeight(), r.getWidth()) - 16));
		r.removeFromLeft(jmin(font.getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 60));
		r.removeFromLeft(2);
	}
	
	xSlider->setBounds(r.removeFromLeft(r.getWidth() / 2 - 5));
	ySlider->setBounds(r.removeFromRight(r.getWidth() - 10));
}

void DoubleSliderUI::showEditWindowInternal()
{
	AlertWindow nameWindow("Change point 2D params", "Set new values and bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; i++) nameWindow.addTextEditor("val" + String(i), String((float)p2d->value[i]), "Value " + coordNames[i]);

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		float newVals[2];
		for (int i = 0; i < 2; i++) newVals[i] = nameWindow.getTextEditorContents("val" + String(i)).getFloatValue();
		p2d->setPoint(newVals[0], newVals[1]);
	}
}

void DoubleSliderUI::showEditRangeWindowInternal()
{
	if (!parameter->isCustomizableByUser) return;

	AlertWindow nameWindow("Change point 2D params", "Set new values and bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; i++)
	{
		nameWindow.addTextEditor("minVal" + String(i), String((float)p2d->minimumValue[i]), "Minimum " + coordNames[i]);
		nameWindow.addTextEditor("maxVal" + String(i), String((float)p2d->maximumValue[i]), "Maximum " + coordNames[i]);
	}

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		float newMins[2];
		float newMaxs[2];
		for (int i = 0; i < 2; i++)
		{
			newMins[i] = nameWindow.getTextEditorContents("minVal" + String(i)).getFloatValue();
			newMaxs[i] = nameWindow.getTextEditorContents("maxVal" + String(i)).getFloatValue();
		}
		p2d->setBounds(newMins[0], newMins[1], jmax(newMins[0], newMaxs[0]), jmax(newMins[1], newMaxs[1]));
	}
}


void DoubleSliderUI::rangeChanged(Parameter * p)
{
	if (p != parameter) return;
	xParam.setRange(parameter->minimumValue[0], parameter->maximumValue[0]);
	yParam.setRange(parameter->minimumValue[1], parameter->maximumValue[1]);
}


void DoubleSliderUI::feedbackStateChanged()
{
	xParam.setControllableFeedbackOnly(parameter->isControllableFeedbackOnly);
	yParam.setControllableFeedbackOnly(parameter->isControllableFeedbackOnly);
}

void DoubleSliderUI::newMessage(const Parameter::ParameterEvent & e)
{
	ParameterUI::newMessage(e);

	if (e.parameter == parameter)
	{
		xParam.setValue(((Point2DParameter*)e.parameter)->x);
		yParam.setValue(((Point2DParameter*)e.parameter)->y);
	}
	else if (isInteractable())
	{
		if (e.parameter == &xParam)
		{
			if (xParam.floatValue() != p2d->x) p2d->setPoint(xParam.floatValue(), yParam.floatValue());
		}
		else if (e.parameter == &yParam)
		{
			if (yParam.floatValue() != p2d->y) p2d->setPoint(xParam.floatValue(), yParam.floatValue());
		}
	}
}
