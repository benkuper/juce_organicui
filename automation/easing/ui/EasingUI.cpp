/*
  ==============================================================================

    EasingUI.cpp
    Created: 16 Dec 2016 7:13:11pm
    Author:  Ben

  ==============================================================================
*/

#pragma warning(disable:4244)

EasingUI::EasingUI(Easing * e, Colour color) :
	InspectableContentComponent(e),
	easing(e),
	color(color),
	viewValueRange(0,1)
{
	autoDrawContourWhenSelected = false;
	
	bringToFrontOnSelect = false; 
	setWantsKeyboardFocus(false);
	setMouseClickGrabsKeyboardFocus(false);

	setRepaintsOnMouseActivity(true);
	easing->addAsyncContainerListener(this);
}

EasingUI::~EasingUI()
{
	if(!easing.wasObjectDeleted()) easing->removeAsyncContainerListener(this);
}

void EasingUI::setViewValueRange(const Point<float> range, bool updatePath)
{
	//if (viewValueRange == range) return;
	
	p1 = Point<int>(0, getYForValue(easing->getValue(0)));
	p2 = Point<int>(getWidth(), getYForValue(easing->getValue(1)));
	
	viewValueRange.setXY(range.x, range.y);
	if(updatePath) generatePath();
}

void EasingUI::paint(Graphics &g)
{
	
	Colour c = color;
	if (easing.wasObjectDeleted()) return;
	if (easing->isSelected) c = HIGHLIGHT_COLOR;
	if (isMouseOver()) c = c.brighter();

	g.setColour(c);
	g.strokePath(drawPath, PathStrokeType(isMouseOver()?2:1));

	//g.setColour(Colours::purple);
	//g.strokePath(hitPath, PathStrokeType(2));

	paintInternal(g);
	
}

void EasingUI::resized()
{
	setViewValueRange(viewValueRange); //force update range
}

void EasingUI::generatePath()
{
	
	drawPath.clear();
	drawPath.startNewSubPath(p1.x, p1.y); 
	generatePathInternal();

	if(drawPath.getLength()) buildHitPath();
	
}

void EasingUI::generatePathInternal()
{
	autoGeneratePathWithPrecision();
}

void EasingUI::autoGeneratePathWithPrecision(int precision)
{
	if (getHeight() == 0) return;
	if (precision == 0) precision = getWidth();
	else precision = jmin(getWidth(), precision);

	jassert(viewValueRange.x != viewValueRange.y);

	for (int i = 1; i <= precision; i++)
	{
		float t = i*1.f / precision;
		//float v1 = 1 - (y1*1.f / getHeight());
		//float v2 = 1 - (y2*1.f / getHeight());
		float val = easing->getValue(t);
		drawPath.lineTo(t * getWidth(), getYForValue(val));
		
	}
}

void EasingUI::buildHitPath()
{
	Array<Point<float>> hitPoints;
	
	for (int i = 0; i <= hitPathPrecision; i++)
	{
		hitPoints.add(drawPath.getPointAlongPath(drawPath.getLength()*i / (hitPathPrecision -1)));
	}

	const float margin = 5;

	hitPath.clear();
	Array<Point<float>> firstPoints;
	Array<Point<float>> secondPoints;
	for (int i = 0; i < hitPathPrecision; i++)
	{
		Point<float> tp;
		Point<float> sp;
		if (i == 0 || i == hitPathPrecision - 1)
		{
			tp = hitPoints[i].translated(0, -margin);
			sp = hitPoints[i].translated(0, margin);
		}
		else
		{
			float angle1 = 0; 
			float angle2 = 0;
			/*if (hitPoints[i].x == hitPoints[i - 1].x) angle1 = 0;
			else if (hitPoints[i].y == hitPoints[i - 1].y) angle1 = -float_Pi / 4;
			else */ angle1 = hitPoints[i].getAngleToPoint(hitPoints[i - 1]);

			/*if (hitPoints[i].x == hitPoints[i - 1].x) angle1 = 0;
			else if (hitPoints[i].y == hitPoints[i - 1].y) angle1 = -float_Pi / 4;
			else */angle2 = hitPoints[i].getAngleToPoint(hitPoints[i + 1]);

		
			if (angle1 < 0) angle1 += float_Pi * 2;
			if (angle2 < 0) angle2 += float_Pi * 2;

			float angle = (angle1 + angle2) / 2.f;

			if (angle1 < angle2) angle += float_Pi;

			tp = hitPoints[i].getPointOnCircumference(margin, angle + float_Pi);
			sp = hitPoints[i].getPointOnCircumference(margin, angle);
		}

		firstPoints.add(tp);
		secondPoints.insert(0, sp);
	}

	if (firstPoints.size() > 1 && !std::isnan(firstPoints[0].x))
	{
		hitPath.startNewSubPath(firstPoints[0]);
		for (int i = 1; i < firstPoints.size(); i++)
		{
			if (!std::isnan(firstPoints[i].x)) hitPath.lineTo(firstPoints[i]);
		}
		for (int i = 0; i < secondPoints.size(); i++)
		{
			if (!std::isnan(firstPoints[i].x)) hitPath.lineTo(secondPoints[i]);
		}
		hitPath.closeSubPath();
	}
}

int EasingUI::getYForValue(float value)
{
	return jmap<float>(value, viewValueRange.x, viewValueRange.y, getHeight(), 0);
}

float EasingUI::getValueForY(int y)
{
	return jmap<float>(y, getHeight(), 0, viewValueRange.x, viewValueRange.y);
}

bool EasingUI::hitTest(int tx, int ty)
{
	return hitPath.contains((float)tx, (float)ty);
}

/*
void EasingUI::resized()
{
	//
}
*/

void EasingUI::newMessage(const ContainerAsyncEvent & e)
{
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		easingControllableFeedbackUpdate(e.targetControllable);
	}
}



LinearEasingUI::LinearEasingUI(LinearEasing * e) :
	EasingUI(e)
{
	
}

void LinearEasingUI::generatePathInternal()
{
	
	drawPath.lineTo(p2.x, p2.y);
}

HoldEasingUI::HoldEasingUI(HoldEasing * e) :
	EasingUI(e)
{

}

void HoldEasingUI::generatePathInternal()
{
	drawPath.lineTo(p2.x, p1.y);
	drawPath.lineTo(p2.x, p2.y);

}

CubicEasingUI::CubicEasingUI(CubicEasing * e) :
	EasingUI(e)
{
	addChildComponent(h1);
	addChildComponent(h2);
	h1.setVisible(easing->isSelected);
	h2.setVisible(easing->isSelected);

	h1.addMouseListener(this,false);
	h2.addMouseListener(this,false);
}

bool CubicEasingUI::hitTest(int tx, int ty)
{
	bool result = EasingUI::hitTest(tx, ty);

	if(easing->isSelected)
	{
		result |= h1.getLocalBounds().contains(h1.getMouseXYRelative());
		result |= h2.getLocalBounds().contains(h2.getMouseXYRelative());
	}

	return result;
}

void CubicEasingUI::resized()
{
	CubicEasing * ce = static_cast<CubicEasing *>(easing.get());

	Point<int> a = Point<int>(p1.x+(ce->k1Anchor->x / ce->length)*getWidth(), getYForValue(ce->getValue(0) + ce->k1Anchor->y));
	Point<int> b = Point<int>(p2.x-(ce->k2Anchor->x / ce->length)*getWidth(), getYForValue(ce->getValue(1) + ce->k2Anchor->y));
	 
	h1.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(a));
	h2.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(b));

	EasingUI::resized();
}

void CubicEasingUI::generatePathInternal()
{
	CubicEasing* ce = static_cast<CubicEasing*>(easing.get());

	Point<int> a = Point<int>(p1.x + (ce->k1Anchor->x / ce->length) * getWidth(), getYForValue(ce->getValue(0) + ce->k1Anchor->y));
	Point<int> b = Point<int>(p2.x - (ce->k2Anchor->x / ce->length) * getWidth(), getYForValue(ce->getValue(1) + ce->k2Anchor->y));

	drawPath.cubicTo(a.toFloat(), b.toFloat(), p2.toFloat());
}

void CubicEasingUI::paintInternal(Graphics & g)
{ 
	if (!easing->isSelected) return;

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);

	g.drawLine(p1.x, p1.y, h1.getBounds().getCentreX(), h1.getBounds().getCentreY());
	g.drawLine(p2.x, p2.y, h2.getBounds().getCentreX(), h2.getBounds().getCentreY());
	
}

void CubicEasingUI::inspectableSelectionChanged(Inspectable *)
{
	if (easing.wasObjectDeleted()) return;
	h1.setVisible(easing->isSelected);
	h2.setVisible(easing->isSelected);
	resized();
	repaint();
}

void CubicEasingUI::easingControllableFeedbackUpdate(Controllable * c)
{
	CubicEasing * ce = static_cast<CubicEasing *>(easing.get());
	if (c == ce->k1Anchor || c == ce->k2Anchor)
	{
		resized();
		repaint();
	}
}

void CubicEasingUI::mouseDrag(const MouseEvent & e)
{
	if (e.eventComponent == &h1 || e.eventComponent == &h2)
	{
		CubicEasing * ce = static_cast<CubicEasing *>(easing.get());
		
		Point2DParameter * targetAnchor = (e.eventComponent == &h1) ? ce->k1Anchor : ce->k2Anchor;
		Point<int> mp = e.getEventRelativeTo(this).getPosition();
		
		float relX = mp.x * 1.0f / getWidth();
		float val = getValueForY(mp.y);
		if (targetAnchor == ce->k2Anchor)
		{
			relX = 1 - relX;
			val -= ce->getValue(1);
		}
		else
		{
			val -= ce->getValue(0);
		}

		Point<float> targetPoint = Point<float>(relX * easing->length, val);
		targetAnchor->setPoint(targetPoint);
	}
	else
	{
		CubicEasing * ce = static_cast<CubicEasing *>(easing.get());

		Point<int> mp = e.getEventRelativeTo(this).getPosition();
		Point<int> mp1;
		Point<int> mp2;
		
		if (e.mods.isShiftDown())
		{
			mp1 = Point<int>(mp.x, p1.y);
			mp2 = Point<int>(p2.x - mp.x, p2.y);
		}
		else if (e.mods.isAltDown())
		{
			mp1 = Point<int>(p1.x, mp.y);
			mp2 = Point<int>(p2.x, p1.y + (p2.y-mp.y));
		}
		else
		{
			mp1 = (p1 + mp) / 2;
			mp2 = (p2 + mp) / 2;
		}
		
		Point<float> t1 = Point<float>((mp1.x * 1.f / getWidth()) * easing->length, getValueForY(mp1.y) - easing->getValue(0));
		Point<float> t2 = Point<float>((1 - (mp2.x * 1.f / getWidth())) * easing->length, getValueForY(mp2.y) - easing->getValue(1));

		ce->k1Anchor->setPoint(t1);
		ce->k2Anchor->setPoint(t2);
	}
}


EasingUI::EasingHandle::EasingHandle() 
{
	setRepaintsOnMouseActivity(true);
}

void EasingUI::EasingHandle::paint(Graphics & g)
{
	Colour c =LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter(.8f);
	g.setColour(c);
	g.fillEllipse(getLocalBounds().reduced(isMouseOver()?3:5).toFloat());
}

SineEasingUI::SineEasingUI(SineEasing * e) :
	EasingUI(e)
{
	hitPathPrecision = jmin<float>(1 / e->freqAmp->floatValue(), 100);
	generatePath();

	addChildComponent(h1);
	h1.setVisible(easing->isSelected);
	h1.addMouseListener(this, false);
}

bool SineEasingUI::hitTest(int tx, int ty)
{
	bool result = EasingUI::hitTest(tx, ty);

	if (easing->isSelected)
	{
		result |= h1.getLocalBounds().contains(h1.getMouseXYRelative());
	}

	return result;
}

void SineEasingUI::resized()
{
	SineEasing * ce = static_cast<SineEasing *>(easing.get());

	Point<int> a = p1 + Point<int>(ce->freqAmp->x * getWidth() / easing->length, getYForValue(ce->freqAmp->y));

	h1.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(a.toInt()));

	EasingUI::resized();

}

void SineEasingUI::paintInternal(Graphics & g)
{
	if (!easing->isSelected) return;

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);

	Point<int> hp = h1.getBounds().getCentre();
	g.drawLine(p1.x, p1.y, hp.x, p1.y, 2);
	g.drawLine(hp.x, p1.y, hp.x, hp.y, 2);
}

void SineEasingUI::inspectableSelectionChanged(Inspectable *)
{
	if (easing.wasObjectDeleted()) return;
	h1.setVisible(easing->isSelected);
	resized();
	repaint();
}

void SineEasingUI::easingControllableFeedbackUpdate(Controllable  * c)
{
	SineEasing * ce = static_cast<SineEasing *>(easing.get());
	if (c == ce->freqAmp)
	{
		hitPathPrecision = jmin<float>(10 / ce->freqAmp->floatValue(),100);
		resized();
		repaint();
	}
}

void SineEasingUI::mouseDrag(const MouseEvent & e)
{
	if (e.eventComponent == &h1)
	{
		CubicEasing * ce = static_cast<CubicEasing *>(easing.get());

		Point2DParameter * targetAnchor = (e.eventComponent == &h1) ? ce->k1Anchor : ce->k2Anchor;
		Point<int> mp = e.getEventRelativeTo(this).getPosition();

		Point<float> targetPoint = Point<float>(mp.x*1.f / getWidth(), getValueForY(mp.y)-p1.y);
		targetAnchor->setPoint(targetPoint);
	}
}
