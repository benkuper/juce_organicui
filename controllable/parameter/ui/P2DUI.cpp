P2DUI::P2DUI(Point2DParameter* parameter) :
	ParameterUI(parameter),
	p2d(parameter),
	shouldRepaint(true),
#if JUCE_MAC
	updateRate(15)
#else
	updateRate(30)
#endif
{
	setRepaintsOnMouseActivity(true);
}

P2DUI::~P2DUI()
{
}

void P2DUI::mouseDownInternal(const MouseEvent&)
{
	mouseDownValue = parameter->getValue();
	mouseDownNormalizedValue = parameter->getNormalizedValue();
	setMouseCursor(MouseCursor::NoCursor);
	repaint();
}

void P2DUI::mouseDrag(const MouseEvent& e)
{
	if (mouseDownNormalizedValue.isVoid()) return;

	float sensitivity = e.mods.isAltDown() ? .5f : 1;

	float dx = e.mods.isAltDown() ? 0 : e.getDistanceFromDragStartX() * sensitivity / getWidth();
	float dy = e.mods.isShiftDown() ? 0 : -e.getDistanceFromDragStartY() * sensitivity / getHeight();

	var val;
	val.append((float)mouseDownNormalizedValue[0] + dx);
	val.append((float)mouseDownNormalizedValue[1] + dy);

	p2d->setNormalizedValue(val);
}

void P2DUI::mouseUpInternal(const MouseEvent&)
{
	p2d->setUndoableValue(mouseDownValue, p2d->value);
	mouseDownValue = var();
	mouseDownNormalizedValue = var();
	setMouseCursor(MouseCursor::NormalCursor);
	repaint();
}

void P2DUI::visibilityChanged()
{
	if (isVisible()) startTimerHz(updateRate); //30 fps for slider
	else stopTimer();
}

void P2DUI::paint(Graphics& g)
{
	if (parameter == nullptr || parameter.wasObjectDeleted()) return;
	Rectangle<int> r = getLocalBounds().reduced(2);

	float radius = 4.5f;
	Colour bgColor = useCustomBGColor ? customBGColor : BG_COLOR;

	g.setColour(bgColor);
	g.fillRoundedRectangle(r.toFloat(), 2);

	g.setColour(bgColor.brighter(.2f));
	g.drawRoundedRectangle(r.toFloat(), 2, customContourThickness);

	Point<float> p = p2d->getPoint();

	var relVal = p2d->getNormalizedValue();
	Point<float> relP(relVal[0], relVal[1]);

	if (showLabel || showValue)
	{
		g.setColour(bgColor.brighter(.15f));
		if (showLabel) g.drawFittedText(parameter->niceName, r, Justification::centred, 3);
		if (showValue)
		{
			String xt = String::formatted("%." + String(3 + 1) + "f", p.x);
			String yt = String::formatted("%." + String(3 + 1) + "f", p.y);

			g.drawFittedText(xt, r.withTop(r.getBottom() - 16), Justification::centred, 3);

			juce::AffineTransform at;
			at = at.rotated((float)(-double_Pi / 2.0f));// , sliderBounds.getCentreX(), sliderBounds.getCentreY());
			at = at.translated((float)r.getWidth() - 16, (float)getHeight());
			g.addTransform(at);
			g.drawFittedText(yt, Rectangle<int>(0, 0, r.getHeight(), 16), Justification::centred, 3);
			g.addTransform(at.inverted());
		}
	}

	Point<float> rp = r.toFloat().reduced(radius, radius).getRelativePoint(relP.x, 1 - relP.y);

	if (isMouseOverOrDragging())
	{
		g.setColour(bgColor.brighter(.1f));
		g.drawHorizontalLine(rp.y, 0, getWidth());
		g.drawVerticalLine(rp.x, 0, getHeight());
	}

	Rectangle<float> pr = Rectangle<float>(0, 0, radius * 2, radius * 2).withCentre(rp);
	Colour fgColor = useCustomFGColor ? customFGColor : (isInteractable() ? GREEN_COLOR : FEEDBACK_COLOR);
	if (isInteractable())
	{
		if (isMouseButtonDown()) fgColor = HIGHLIGHT_COLOR;
		else if (isMouseOver()) fgColor = fgColor.brighter();
	}

	g.setColour(fgColor);
	g.fillEllipse(pr.toFloat());

}

void P2DUI::valueChanged(const var& v)
{
	shouldRepaint = true;

}

void P2DUI::rangeChanged(Parameter* p)
{
	shouldRepaint = true;
}



void P2DUI::showEditWindowInternal()
{
	AlertWindow nameWindow("Change point 2D values", "Set new values for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; ++i) nameWindow.addTextEditor("val" + String(i), String((float)p2d->value[i]), "Value " + coordNames[i]);

	nameWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = nameWindow.runModalLoop();

	if (result)
	{
		float newVals[2];
		for (int i = 0; i < 2; ++i) newVals[i] = nameWindow.getTextEditorContents("val" + String(i)).getFloatValue();
		p2d->setUndoablePoint(p2d->x, p2d->y, newVals[0], newVals[1]);
	}
}

void P2DUI::showEditRangeWindowInternal()
{
	if (!parameter->isCustomizableByUser) return;

	AlertWindow nameWindow("Change point 2D bounds", "Set new bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; ++i)
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
		for (int i = 0; i < 2; ++i)
		{
			newMins[i] = nameWindow.getTextEditorContents("minVal" + String(i)).getFloatValue();
			newMaxs[i] = nameWindow.getTextEditorContents("maxVal" + String(i)).getFloatValue();
		}
		p2d->setBounds(newMins[0], newMins[1], jmax(newMins[0], newMaxs[0]), jmax(newMins[1], newMaxs[1]));
	}
}

void P2DUI::timerCallback()
{
	if (shouldRepaint)
	{
		repaint();
		shouldRepaint = false;
	}
}
