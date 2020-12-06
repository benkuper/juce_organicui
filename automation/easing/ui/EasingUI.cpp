#include "EasingUI.h"
/*
  ==============================================================================

	EasingUI.cpp
	Created: 16 Dec 2016 7:13:11pm
	Author:  Ben

  ==============================================================================
*/

#pragma warning(disable:4244)

EasingUI::EasingUI(Easing* e) :
	InspectableContentComponent(e),
	easing(e),
	showFirstHandle(false),
	showLastHandle(false)
{
	autoDrawContourWhenSelected = false;

	bringToFrontOnSelect = false;

	setRepaintsOnMouseActivity(true);
	easing->addAsyncContainerListener(this);
}

EasingUI::~EasingUI()
{
	if (!easing.wasObjectDeleted()) easing->removeAsyncContainerListener(this);
}

void EasingUI::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;
	//g.fillAll(Colours::purple.withAlpha(.2f));

	Colour c = NORMAL_COLOR;
	if (easing.wasObjectDeleted()) return;
	if (easing->isSelected) c = HIGHLIGHT_COLOR;
	if (isMouseOver()) c = c.brighter();

	ColourGradient gr;
	//const int precision = 10;

	g.setColour(c);
	g.strokePath(drawPath, PathStrokeType(isMouseOver() ? 2 : 1));

	//g.setColour(Colours::lightgoldenrodyellow);
	//g.strokePath(hitPath, PathStrokeType(isMouseOver() ? 2 : 1));


	//g.setColour(Colours::pink);
	//for (int i = 0; i < 100; ++i)
	//{
	//	float p = i * 1.0f / 100;
	//	float t = easing->start.x + p * easing->length;
	//	g.fillEllipse(Rectangle<int>(0, 0, 4, 4).withCentre(getUIPosForValuePos(Point<float>(t, easing->getValue(p)))).toFloat());
	//}
	
	paintInternal(g);

}

void EasingUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;
	generatePath();
}

void EasingUI::generatePath()
{
	drawPath.clear();
	hitPath.clear();

	if (getHeight() == 0 || getWidth() == 0) return;

	drawPath.startNewSubPath(getUIPosForValuePos(easing->start).toFloat());

	if (valueBounds.isEmpty() || (easing->start == easing->end)) return;

	generatePathInternal();
	if (drawPath.getLength()) buildHitPath();
}

void EasingUI::generatePathInternal()
{
	autoGeneratePathWithPrecision(getWidth()/2);
}

void EasingUI::autoGeneratePathWithPrecision(int precision)
{
	if (getHeight() == 0) return;
	if (precision == 0) precision = getWidth();
	else precision = jmin(getWidth(), precision);

	for (int i = 1; i <= precision; ++i)
	{
		float t = i * 1.f / precision;
		float v = easing->getValue(t);
		float tx = easing->start.x + t * easing->length;
		Point<int> pv = getUIPosForValuePos(Point<float>(tx, v));
		//if (pv.y > valueBounds.getBottom() + 10 || pv.y > valueBounds.getY() - 10) continue;
		drawPath.lineTo(pv.toFloat());
	}
}

void EasingUI::buildHitPath()
{
	Array<Point<float>> hitPoints;

	for (int i = 0; i <= hitPathPrecision; ++i)
	{
		hitPoints.add(drawPath.getPointAlongPath(drawPath.getLength() * i / (hitPathPrecision - 1)));
	}

	const float margin = 5;

	hitPath.clear();
	Array<Point<float>> firstPoints;
	Array<Point<float>> secondPoints;
	for (int i = 0; i < hitPathPrecision; ++i)
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
		for (int i = 1; i < firstPoints.size(); ++i)
		{
			if (!std::isnan(firstPoints[i].x)) hitPath.lineTo(firstPoints[i]);
		}
		for (int i = 0; i < secondPoints.size(); ++i)
		{
			if (!std::isnan(firstPoints[i].x)) hitPath.lineTo(secondPoints[i]);
		}
		hitPath.closeSubPath();
	}
}

bool EasingUI::hitTest(int tx, int ty)
{
	Point<int> p(tx, ty);
	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	return p.getDistanceFrom(p1) > 16 && p.getDistanceFrom(p2) > 16 && hitPath.contains((float)tx, (float)ty);
}

void EasingUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	showFirstHandle = showFirst;
	showLastHandle = showLast;
}


void EasingUI::newMessage(const ContainerAsyncEvent& e)
{
	if (e.targetControllable.wasObjectDeleted()) return; 
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		easingControllableFeedbackUpdate(e.targetControllable);
		repaint();
	}
}


void EasingUI::setValueBounds(const Rectangle<float> _valueBounds)
{
	valueBounds = _valueBounds;
	resized();
}


Point<int> EasingUI::getUIPosForValuePos(const Point<float>& valuePos) const
{
	if (inspectable.wasObjectDeleted()) return Point<int>();
	return getLocalBounds().getRelativePoint((valuePos.x - valueBounds.getX()) / valueBounds.getWidth(), 1 - ((valuePos.y - valueBounds.getY()) / valueBounds.getHeight()));
}

Point<float> EasingUI::getValuePosForUIPos(const Point<int>& uiPos) const
{
	if (inspectable.wasObjectDeleted()) return Point<float>();
	return valueBounds.getRelativePoint(uiPos.x * 1.0f / getWidth(), 1- (uiPos.y * 1.0f / getHeight()));
}

// EASINGS
LinearEasingUI::LinearEasingUI(LinearEasing* e) :
	EasingUI(e)
{

}

void LinearEasingUI::generatePathInternal()
{
	drawPath.lineTo(getUIPosForValuePos(easing->end).toFloat());
}

CubicEasingUI::CubicEasingUI(CubicEasing* e) :
	EasingUI(e),
	ce(e),
	syncHandles(false)
{
	addChildComponent(h1);
	addChildComponent(h2);
	h1.setVisible(showFirstHandle);
	h2.setVisible(showLastHandle);

	h1.addMouseListener(this, false);
	h2.addMouseListener(this, false);
}

bool CubicEasingUI::hitTest(int tx, int ty)
{
	bool result = EasingUI::hitTest(tx, ty);

	if (showFirstHandle)
	{
		result |= h1.getLocalBounds().contains(h1.getMouseXYRelative());
	}

	if (showLastHandle)
	{
		result |= h2.getLocalBounds().contains(h2.getMouseXYRelative());
	}

	return result;
}

void CubicEasingUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;

	//Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	//Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	Point<int> a = getUIPosForValuePos(easing->start + ce->anchor1->getPoint());
	Point<int> b = getUIPosForValuePos(easing->end + ce->anchor2->getPoint());

	h1.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(a));
	h2.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(b));

	EasingUI::resized();
}

void CubicEasingUI::generatePathInternal()
{
	//Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	Point<int> a = getUIPosForValuePos(easing->start + ce->anchor1->getPoint());
	Point<int> b = getUIPosForValuePos(easing->end + ce->anchor2->getPoint());

	drawPath.cubicTo(a.toFloat(), b.toFloat(), p2.toFloat());
}

void CubicEasingUI::paintInternal(Graphics& g)
{

	if (!showFirstHandle && !showLastHandle) return;

	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);

	if (showFirstHandle) g.drawLine(p1.x, p1.y, h1.getBounds().getCentreX(), h1.getBounds().getCentreY());
	if (showLastHandle) g.drawLine(p2.x, p2.y, h2.getBounds().getCentreX(), h2.getBounds().getCentreY());

}

void CubicEasingUI::easingControllableFeedbackUpdate(Controllable* c)
{
	if (c == ce->anchor1 || c == ce->anchor2)
	{
		resized();
		repaint();
	}
}

void CubicEasingUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	EasingUI::setShowEasingHandles(showFirst, showLast);
	h1.setVisible(showFirstHandle);
	h2.setVisible(showLastHandle);
	resized();
	repaint();
}

void CubicEasingUI::mouseDown(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	EasingUI::mouseDown(e);

	if (!e.mods.isCommandDown())
	{
		h1ValueAtMouseDown = ce->anchor1->getPoint();
		h2ValueAtMouseDown = ce->anchor2->getPoint();
	}
}

void CubicEasingUI::mouseDrag(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	syncHandles = !e.mods.isAltDown();

	if (e.eventComponent == &h1 || e.eventComponent == &h2)
	{
		Point2DParameter* targetAnchor = (e.eventComponent == &h1) ? ce->anchor1 : ce->anchor2;
		Point<float> targetRefPoint = (e.eventComponent == &h1) ? ce->start : ce->end;

		Point<float> targetPoint = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition()); //Point<float>(mp.x * 1.f / getWidth(), jmap<float>(mp.y, y1, y2, 0, 1));
		targetAnchor->setPoint(targetPoint - targetRefPoint);
	}
	else
	{
		if (e.mods.isShiftDown())
		{
			syncHandles = false;
			
			if (e.mods.isAltDown())
			{
				//Point<int> p1 = getUIPosForValuePos(easing->start);
				Point<float> mVal = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition());

				float ty = mVal.y - easing->start.y;
				ce->anchor1->setPoint(0, ty);
				ce->anchor2->setPoint(0, -ty);
			}
			else
			{
				//Point<int> p1 = getUIPosForValuePos(easing->start);
				Point<float> mVal = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition());

				float tx = mVal.x - easing->start.x;
				ce->anchor1->setPoint(tx, 0);
				ce->anchor2->setPoint(-tx, 0);
			}
		}else
		{
			//Point<int> p1 = getUIPosForValuePos(easing->start);
			//Point<int> p2 = getUIPosForValuePos(easing->end);
			Point<float> mVal = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition());

			ce->anchor1->setPoint((mVal - easing->start) / 2);
			ce->anchor2->setPoint((mVal - easing->end) / 2);
		}
	}
		
}

void CubicEasingUI::mouseUp(const MouseEvent& e)
{
	EasingUI::mouseUp(e);

	Array<UndoableAction*> actions;
	actions.add(ce->anchor1->setUndoablePoint(h1ValueAtMouseDown, ce->anchor1->getPoint(), true));
	actions.add(ce->anchor2->setUndoablePoint(h2ValueAtMouseDown, ce->anchor2->getPoint(), true));
	UndoMaster::getInstance()->performActions("Move anchors", actions);
}

// HANDLES
EasingUI::EasingHandle::EasingHandle()
{
	setRepaintsOnMouseActivity(true);
}

void EasingUI::EasingHandle::paint(Graphics& g)
{
	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter(.8f);
	g.setColour(c);
	g.fillEllipse(getLocalBounds().reduced(isMouseOver() ? 4 : 6).toFloat());
}

HoldEasingUI::HoldEasingUI(HoldEasing* e) :
	EasingUI(e)
{
}

void HoldEasingUI::generatePathInternal()
{
	drawPath.lineTo(getUIPosForValuePos(easing->end.withY(easing->start.y)).toFloat());
	drawPath.lineTo(getUIPosForValuePos(easing->end).toFloat());
}

SineEasingUI::SineEasingUI(SineEasing* e) :
	EasingUI(e),
	se(e)
{
	addChildComponent(h1);
	h1.addMouseListener(this, false);
}

bool SineEasingUI::hitTest(int tx, int ty)
{
	bool result = EasingUI::hitTest(tx, ty);

	if (showFirstHandle && showLastHandle) result |= h1.getLocalBounds().contains(h1.getMouseXYRelative());
	return result;
}

void SineEasingUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;
	hitPathPrecision = getWidth() / 4;

	//Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> a = getUIPosForValuePos(easing->start + se->freqAmp->getPoint());
	h1.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(a));

	EasingUI::resized();
}

void SineEasingUI::paintInternal(Graphics& g)
{
	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);

	if (showFirstHandle && showLastHandle) g.drawLine(p1.x, p1.y, h1.getBounds().getCentreX(), h1.getBounds().getCentreY());
}

void SineEasingUI::easingControllableFeedbackUpdate(Controllable* c)
{
	if (c == se->freqAmp)
	{
		resized();
		repaint();
	}
}

void SineEasingUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	EasingUI::setShowEasingHandles(showFirst, showLast);
	h1.setVisible(showFirstHandle && showLastHandle);
	resized();
	repaint();
}

void SineEasingUI::mouseDown(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	EasingUI::mouseDown(e);

	if (!e.mods.isCommandDown())
	{
		h1ValueAtMouseDown = se->freqAmp->getPoint();
	}
}

void SineEasingUI::mouseDrag(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;

	if (e.eventComponent == &h1)
	{
		Point<float> targetPoint = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition()); //Point<float>(mp.x * 1.f / getWidth(), jmap<float>(mp.y, y1, y2, 0, 1));
		se->freqAmp->setPoint(targetPoint - se->start);
	}
}

void SineEasingUI::mouseUp(const MouseEvent& e)
{
	se->freqAmp->setUndoablePoint(h1ValueAtMouseDown, se->freqAmp->getPoint());

}
