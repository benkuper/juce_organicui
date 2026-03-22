using namespace juce;

P2DUI::P2DUI(Array<Point2DParameter*>parameters) :
	ParameterUI(Inspectable::getArrayAs<Point2DParameter, Parameter>(parameters), ORGANICUI_DEFAULT_TIMER),
	p2ds(parameters),
	p2d(parameters[0])
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

	float dx = e.mods.isAltDown() ? 0 : e.getDistanceFromDragStartX() * sensitivity / canvasRect.getWidth();
	float dy = e.mods.isShiftDown() ? 0 : -e.getDistanceFromDragStartY() * sensitivity / canvasRect.getHeight();
	if (p2d->extendedEditorInvertX) dx = -dx;
	if (p2d->extendedEditorInvertY) dy = -dy;

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

void P2DUI::paint(Graphics& g)
{
	if (parameter == nullptr || parameter.wasObjectDeleted()) return;

	float radius = 4.5f;
	Colour bgColor = useCustomBGColor ? customBGColor : BG_COLOR;

	g.setColour(bgColor.darker(.3f));
	g.fillRoundedRectangle(getLocalBounds().reduced(1).toFloat(), 2);

	g.setColour(bgColor);
	g.fillRoundedRectangle(canvasRect, 2);

	g.setColour(bgColor.brighter(.2f));
	g.drawRoundedRectangle(canvasRect, 2, customContourThickness);

	Point<float> p = p2d->getPoint();

	var relVal = p2d->getNormalizedValue();
	Point<float> relP(relVal[0], relVal[1]);
	if (p2d->extendedEditorInvertX) relP.setX(1 - relP.x);
	if (p2d->extendedEditorInvertY) relP.setY(1 - relP.y);

	if (showLabel || showValue)
	{
		g.setColour(bgColor.brighter(.15f));
		if (showLabel) g.drawFittedText(parameter->niceName, canvasRect.toNearestInt(), Justification::centred, 3);
		if (showValue)
		{
			String xt = String::formatted("%." + String(3 + 1) + "f", p.x);
			String yt = String::formatted("%." + String(3 + 1) + "f", p.y);

			g.drawFittedText(xt, canvasRect.withTop(canvasRect.getBottom() - 16).toNearestInt(), Justification::centred, 3);

			juce::AffineTransform at;
			at = at.rotated((float)(-MathConstants<double>::pi / 2.0f));// , sliderBounds.getCentreX(), sliderBounds.getCentreY());
			at = at.translated(canvasRect.getRight() - 16, canvasRect.getBottom());
			g.addTransform(at);
			g.drawFittedText(yt, juce::Rectangle<int>(0, 0, canvasRect.getHeight(), 16), Justification::centred, 3);
			g.addTransform(at.inverted());
		}
	}

	Point<float> rp = canvasRect.reduced(radius, radius).getRelativePoint(relP.x, 1 - relP.y);

	if (isMouseOverOrDragging())
	{
		g.setColour(bgColor.brighter(.1f));
		g.drawHorizontalLine(rp.y, 0, getWidth());
		g.drawVerticalLine(rp.x, 0, getHeight());
	}

	juce::Rectangle<float> pr = juce::Rectangle<float>(0, 0, radius * 2, radius * 2).withCentre(rp);
	Colour fgColor = useCustomFGColor ? customFGColor : (isInteractable() ? GREEN_COLOR : FEEDBACK_COLOR);
	if (isInteractable())
	{
		if (isMouseButtonDown()) fgColor = HIGHLIGHT_COLOR;
		else if (isMouseOver()) fgColor = fgColor.brighter();
	}

	g.setColour(fgColor);
	g.fillEllipse(pr.toFloat());

}

void P2DUI::resized()
{
	ParameterUI::resized();

	canvasRect = getLocalBounds().reduced(2).toFloat();

	if (!p2d->extendedEditorStretchMode)
	{
		float rRatio = canvasRect.getAspectRatio();
		Point<float> range((float)p2d->maximumValue[0] - (float)p2d->minimumValue[0], (float)p2d->maximumValue[1] - (float)p2d->minimumValue[1]);
		float pRatio = range.x / range.y;

		if (pRatio > rRatio)
		{
			canvasRect = canvasRect.withSizeKeepingCentre(canvasRect.getWidth(), canvasRect.getWidth() / pRatio);
		}
		else
		{
			canvasRect = canvasRect.withSizeKeepingCentre(canvasRect.getHeight() * pRatio, canvasRect.getHeight());
		}

	}
}

void P2DUI::valueChanged(const var& v)
{
	shouldRepaint = true;

}

void P2DUI::rangeChanged(Parameter* p)
{
	resized();
	shouldRepaint = true;
}


void P2DUI::addPopupMenuItemsInternal(PopupMenu* m)
{
	m->addItem(-100, "Stretch Mode", true, p2d->extendedEditorStretchMode);
	m->addItem(-101, "Invert X", true, p2d->extendedEditorInvertX);
	m->addItem(-102, "Invert Y", true, p2d->extendedEditorInvertY);
}

void P2DUI::handleMenuSelectedID(int result)
{
	ParameterUI::handleMenuSelectedID(result);

	switch (result)
	{
	case -100: p2d->extendedEditorStretchMode = !p2d->extendedEditorStretchMode; break;
	case -101: p2d->extendedEditorInvertX = !p2d->extendedEditorInvertX; break;
	case -102: p2d->extendedEditorInvertY = !p2d->extendedEditorInvertY; break;
	}

	resized();
	shouldRepaint = true;
}

void P2DUI::showEditWindowInternal()
{
	AlertWindow* nameWindow = new AlertWindow("Change point 2D values", "Set new values for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; ++i) nameWindow->addTextEditor("val" + String(i), String((float)p2d->value[i]), "Value " + coordNames[i]);

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	nameWindow->enterModalState(true, ModalCallbackFunction::create([this, &nameWindow](int result)
		{

			if (result)
			{
				float newVals[2];
				for (int i = 0; i < 2; ++i) newVals[i] = nameWindow->getTextEditorContents("val" + String(i)).getFloatValue();
				p2d->setUndoablePoint(p2d->x, p2d->y, newVals[0], newVals[1]);
			}
		}
	),
		true
			);
}

void P2DUI::showEditRangeWindowInternal()
{
	if (!parameter->isCustomizableByUser) return;

	AlertWindow* nameWindow = new AlertWindow("Change point 2D bounds", "Set new bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; ++i)
	{
		nameWindow->addTextEditor("minVal" + String(i), String((float)p2d->minimumValue[i]), "Minimum " + coordNames[i]);
		nameWindow->addTextEditor("maxVal" + String(i), String((float)p2d->maximumValue[i]), "Maximum " + coordNames[i]);
	}

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	nameWindow->enterModalState(true, ModalCallbackFunction::create([this, &nameWindow](int result)
		{
			float newMins[2];
			float newMaxs[2];
			for (int i = 0; i < 2; ++i)
			{
				newMins[i] = nameWindow->getTextEditorContents("minVal" + String(i)).getFloatValue();
				newMaxs[i] = nameWindow->getTextEditorContents("maxVal" + String(i)).getFloatValue();
			}
			p2d->setBounds(newMins[0], newMins[1], jmax(newMins[0], newMaxs[0]), jmax(newMins[1], newMaxs[1]));
		}
	), true);
}
