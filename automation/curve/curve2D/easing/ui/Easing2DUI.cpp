/*
  ==============================================================================

	Easing2DUI.cpp
	Created: 16 Dec 2016 7:13:11pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

#pragma warning(disable:4244)

Easing2DUI::Easing2DUI(Easing2D* e) :
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

Easing2DUI::~Easing2DUI()
{
	if (!easing.wasObjectDeleted()) easing->removeAsyncContainerListener(this);
}

void Easing2DUI::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;
	//g.fillAll(Colours::purple.withAlpha(.2f));

	Colour c = NORMAL_COLOR;
	if (easing.wasObjectDeleted()) return;
	if (easing->isSelected) c = HIGHLIGHT_COLOR;
	if (isMouseOver()) c = c.brighter();

	if (focusRange == 0)
	{
		g.setColour(c);
		g.strokePath(drawPath, PathStrokeType(isMouseOver() ? 2 : 1));
	}
	else
	{
		float prevPos = 0;
		for (int i = 0; i <= 50; i++)
		{

			float pos = (i*1.0f / 50) * drawPath.getLength();
			float curvePos = (i*1.0f / 50) * easing->length;

			float d = jlimit<float>(0, 1, 1 - fabsf(curvePos - focusPos) / (focusRange / 2));

			//in-out for better fading
			float sqt = d * d;
			float td = sqt / (2.0f * (sqt - d) + 1.0f);
			g.setColour(c.withAlpha(td));

			if (pos != prevPos) g.drawLine(Line<float>(drawPath.getPointAlongPath(prevPos), drawPath.getPointAlongPath(pos)), isMouseOver() ? 2 : 1);
			prevPos = pos;
		}
	}
	//g.setColour(Colours::purple);
	//g.strokePath(hitPath, PathStrokeType(2));


	paintInternal(g);

}

void Easing2DUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;
	generatePath();
}

void Easing2DUI::setFocus(float relPos, float range)
{
	if (focusPos == relPos && focusRange == range) return;
	focusPos = relPos;
	focusRange = range;
	repaint();
}

void Easing2DUI::generatePath()
{
	drawPath.clear();
	drawPath.startNewSubPath(getUIPosForValuePos(easing->start).toFloat());

	if (valueBounds.isEmpty() || (easing->start == easing->end)) return;

	generatePathInternal();
	if (drawPath.getLength()) buildHitPath();
}

void Easing2DUI::generatePathInternal()
{
	autoGeneratePathWithPrecision();
}

void Easing2DUI::autoGeneratePathWithPrecision(int precision)
{
	if (getHeight() == 0) return;
	if (precision == 0) precision = getWidth();
	else precision = jmin(getWidth(), precision);

	for (int i = 1; i <= precision; ++i)
	{
		float t = i * 1.f / precision;
		Point<float> v = easing->getValue(t);

		Point<int> pv = getUIPosForValuePos(v);
		drawPath.lineTo(pv.toFloat());
	}
}

void Easing2DUI::buildHitPath()
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
			else if (hitPoints[i].y == hitPoints[i - 1].y) angle1 = -MathConstants<float>::pi / 4;
			else */ angle1 = hitPoints[i].getAngleToPoint(hitPoints[i - 1]);

			/*if (hitPoints[i].x == hitPoints[i - 1].x) angle1 = 0;
			else if (hitPoints[i].y == hitPoints[i - 1].y) angle1 = -MathConstants<float>::pi / 4;
			else */angle2 = hitPoints[i].getAngleToPoint(hitPoints[i + 1]);


			if (angle1 < 0) angle1 += MathConstants<float>::pi * 2;
			if (angle2 < 0) angle2 += MathConstants<float>::pi * 2;

			float angle = (angle1 + angle2) / 2.f;

			if (angle1 < angle2) angle += MathConstants<float>::pi;

			tp = hitPoints[i].getPointOnCircumference(margin, angle + MathConstants<float>::pi);
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

bool Easing2DUI::hitTest(int tx, int ty)
{
	Point<int> p(tx, ty);
	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	return p.getDistanceFrom(p1) > 16 && p.getDistanceFrom(p2) > 16 && hitPath.contains((float)tx, (float)ty);
}

void Easing2DUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	showFirstHandle = showFirst;
	showLastHandle = showLast;
}


void Easing2DUI::newMessage(const ContainerAsyncEvent& e)
{
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		if (e.targetControllable.wasObjectDeleted()) return;
		easingControllableFeedbackUpdate(e.targetControllable);
		repaint();
	}
}


void Easing2DUI::setValueBounds(const juce::Rectangle<float> _valueBounds)
{
	valueBounds = _valueBounds;
	resized();
}


Point<int> Easing2DUI::getUIPosForValuePos(const Point<float>& valuePos) const
{
	if (inspectable.wasObjectDeleted()) return Point<int>();
	return getLocalBounds().getRelativePoint((valuePos.x - valueBounds.getX()) / valueBounds.getWidth(), (valuePos.y - valueBounds.getY()) / valueBounds.getHeight());
}

Point<float> Easing2DUI::getValuePosForUIPos(const Point<int>& uiPos) const
{
	if (inspectable.wasObjectDeleted()) return Point<float>();
	return valueBounds.getRelativePoint(uiPos.x * 1.0f / getWidth(), uiPos.y * 1.0f / getHeight());
}

// EASINGS
LinearEasing2DUI::LinearEasing2DUI(LinearEasing2D* e) :
	Easing2DUI(e)
{

}

void LinearEasing2DUI::generatePathInternal()
{
	drawPath.lineTo(getUIPosForValuePos(easing->end).toFloat());
}

CubicEasing2DUI::CubicEasing2DUI(CubicEasing2D* e) :
	Easing2DUI(e),
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

bool CubicEasing2DUI::hitTest(int tx, int ty)
{
	bool result = Easing2DUI::hitTest(tx, ty);

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

void CubicEasing2DUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;

	//Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	//Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	Point<int> a = getUIPosForValuePos(easing->start + ce->anchor1->getPoint());
	Point<int> b = getUIPosForValuePos(easing->end + ce->anchor2->getPoint());

	h1.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(a));
	h2.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(b));

	Easing2DUI::resized();
}

void CubicEasing2DUI::generatePathInternal()
{
	//Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	Point<int> a = getUIPosForValuePos(easing->start + ce->anchor1->getPoint());
	Point<int> b = getUIPosForValuePos(easing->end + ce->anchor2->getPoint());

	drawPath.cubicTo(a.toFloat(), b.toFloat(), p2.toFloat());
}

void CubicEasing2DUI::paintInternal(Graphics& g)
{
	/*	g.setColour(Colours::lightpink);
		for (int i = 0; i < ce->uniformLUT.size(); ++i)
		{
			g.fillEllipse(juce::Rectangle<int>(0, 0, 4, 4).withCentre(getUIPosForValuePos(ce->uniformLUT[i])).toFloat());
		}
		*/

	if (!showFirstHandle && !showLastHandle) return;

	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);

	if (showFirstHandle) g.drawLine(p1.x, p1.y, h1.getBounds().getCentreX(), h1.getBounds().getCentreY());
	if (showLastHandle) g.drawLine(p2.x, p2.y, h2.getBounds().getCentreX(), h2.getBounds().getCentreY());

}

void CubicEasing2DUI::easingControllableFeedbackUpdate(Controllable* c)
{
	if (c == ce->anchor1 || c == ce->anchor2)
	{
		resized();
		repaint();
	}
}

void CubicEasing2DUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	Easing2DUI::setShowEasingHandles(showFirst, showLast);
	h1.setVisible(showFirstHandle);
	h2.setVisible(showLastHandle);
	resized();
	repaint();
}

void CubicEasing2DUI::mouseDown(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	Easing2DUI::mouseDown(e);

	if (!e.mods.isCommandDown())
	{
		h1ValueAtMouseDown = ce->anchor1->getPoint();
		h2ValueAtMouseDown = ce->anchor2->getPoint();
	}
}

void CubicEasing2DUI::mouseDrag(const MouseEvent& e)
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
		//Point<int> p1 = getUIPosForValuePos(easing->start);
		//Point<int> p2 = getUIPosForValuePos(easing->end);
		Point<float> mVal = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition());

		ce->anchor1->setPoint((mVal - easing->start) / 2);
		ce->anchor2->setPoint((mVal - easing->end) / 2);
	}
}

void CubicEasing2DUI::mouseUp(const MouseEvent& e)
{
	Easing2DUI::mouseUp(e);

	Array<UndoableAction*> actions;
	actions.add(ce->anchor1->setUndoablePoint(h1ValueAtMouseDown, ce->anchor1->getPoint(), true));
	actions.add(ce->anchor2->setUndoablePoint(h2ValueAtMouseDown, ce->anchor2->getPoint(), true));
	UndoMaster::getInstance()->performActions("Move anchors", actions);
}

// HANDLES
Easing2DUI::Easing2DHandle::Easing2DHandle()
{
	setRepaintsOnMouseActivity(true);
}

void Easing2DUI::Easing2DHandle::paint(Graphics& g)
{
	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter(.8f);
	g.setColour(c);
	g.fillEllipse(getLocalBounds().reduced(isMouseOver() ? 4 : 6).toFloat());
}
