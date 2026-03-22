/*
  ==============================================================================

	EasingUI.cpp
	Created: 16 Dec 2016 7:13:11pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

#pragma warning(disable:4244)

EasingUI::EasingUI(Easing* e) :
	InspectableContentComponent(e),
	UITimerTarget(ORGANICUI_DEFAULT_TIMER, "EasingUI"),
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

void EasingUI::handlePaintTimerInternal()
{
	repaint();
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
	//	g.fillEllipse(juce::Rectangle<int>(0, 0, 4, 4).withCentre(getUIPosForValuePos(Point<float>(t, easing->getValue(p)))).toFloat());
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
	autoGeneratePathWithPrecision(getWidth() / 2);
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
		shouldRepaint = true;
	}
}


void EasingUI::setValueBounds(const juce::Rectangle<float> _valueBounds)
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
	return valueBounds.getRelativePoint(uiPos.x * 1.0f / getWidth(), 1 - (uiPos.y * 1.0f / getHeight()));
}

//Callout


EasingUI::HandleEditCalloutComponent::HandleEditCalloutComponent(Point2DParameter* param)
{
	paramEditor.reset(new DoubleSliderUI(param));
	paramEditor->canShowExtendedEditor = false;
	paramEditor->showLabel = false;
	addAndMakeVisible(paramEditor.get());
	setSize(160, 20);
}

EasingUI::HandleEditCalloutComponent::~HandleEditCalloutComponent()
{
}

void EasingUI::HandleEditCalloutComponent::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	paramEditor->setBounds(r.removeFromTop(25).reduced(2));
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
    h1(ce->anchor1),
    h2(ce->anchor2),
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
		shouldRepaint = true;
	}
}

void CubicEasingUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	EasingUI::setShowEasingHandles(showFirst, showLast);
	h1.setVisible(showFirstHandle);
	h2.setVisible(showLastHandle);
	resized();
	shouldRepaint = true;
}

void CubicEasingUI::mouseDown(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	EasingUI::mouseDown(e);

	if (!e.mods.isAnyModifierKeyDown())
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
EasingUI::EasingHandle::EasingHandle(Point2DParameter* parameter) :
	parameter(parameter)
{
	setRepaintsOnMouseActivity(true);
}

void EasingUI::EasingHandle::mouseDoubleClick(const MouseEvent& e)
{
	std::unique_ptr<Component> editComponent(new HandleEditCalloutComponent(parameter));
	CallOutBox* box = &CallOutBox::launchAsynchronously(std::move(editComponent), localAreaToGlobal(getLocalBounds()), nullptr);
	box->setArrowSize(8);
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
	se(e),
	h1(se->freqAmp)
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
		shouldRepaint = true;
	}
}

void SineEasingUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	EasingUI::setShowEasingHandles(showFirst, showLast);
	h1.setVisible(showFirstHandle && showLastHandle);
	resized();
	shouldRepaint = true;
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


ElasticEasingUI::ElasticEasingUI(ElasticEasing* e) :
	EasingUI(e),
	se(e),
	h1(se->param)
{
	addChildComponent(h1);
	h1.addMouseListener(this, false);
}

bool ElasticEasingUI::hitTest(int tx, int ty)
{
	bool result = EasingUI::hitTest(tx, ty);

	if (showFirstHandle && showLastHandle) result |= h1.getLocalBounds().contains(h1.getMouseXYRelative());
	return result;
}

void ElasticEasingUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;
	hitPathPrecision = getWidth() / 4;

	//Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> a = getUIPosForValuePos(easing->start + se->param->getPoint());
	h1.setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(a));

	EasingUI::resized();
}

void ElasticEasingUI::paintInternal(Graphics& g)
{
	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);

	if (showFirstHandle && showLastHandle) g.drawLine(p1.x, p1.y, h1.getBounds().getCentreX(), h1.getBounds().getCentreY());
}

void ElasticEasingUI::easingControllableFeedbackUpdate(Controllable* c)
{
	if (c == se->param)
	{
		resized();
		shouldRepaint = true;
	}
}

void ElasticEasingUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	EasingUI::setShowEasingHandles(showFirst, showLast);
	h1.setVisible(showFirstHandle && showLastHandle);
	resized();
	shouldRepaint = true;
}

void ElasticEasingUI::mouseDown(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	EasingUI::mouseDown(e);

	if (!e.mods.isCommandDown())
	{
		h1ValueAtMouseDown = se->param->getPoint();
	}
}

void ElasticEasingUI::mouseDrag(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;

	if (e.eventComponent == &h1)
	{
		Point<float> targetPoint = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition()); //Point<float>(mp.x * 1.f / getWidth(), jmap<float>(mp.y, y1, y2, 0, 1));
		se->param->setPoint(targetPoint - se->start);
	}
}

void ElasticEasingUI::mouseUp(const MouseEvent& e)
{
	se->param->setUndoablePoint(h1ValueAtMouseDown, se->param->getPoint());

}



BounceEasingUI::BounceEasingUI(BounceEasing* e) :
	EasingUI(e),
	se(e)
{
}

void BounceEasingUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;
	hitPathPrecision = getWidth() / 4;
	EasingUI::resized();
}

void BounceEasingUI::paintInternal(Graphics& g)
{
	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);
}




GenericEasingUI::GenericEasingUI(Easing * e, Point2DParameter * a1, Point2DParameter *a2, Array<Parameter *> _extraParams) :
	EasingUI(e)
{
	if (a1 != nullptr)
	{
		h1.reset(new EasingHandle(a1));
		addChildComponent(h1.get());
		h1->setVisible(showFirstHandle);
		h1->addMouseListener(this, false);
	}

	if (a2 != nullptr)
	{
		h2.reset(new EasingHandle(a2));
		addChildComponent(h2.get());
		h2->setVisible(showLastHandle);
		h2->addMouseListener(this, false);
	}

	for (auto& p : _extraParams)
	{
		ControllableUI* cui = p->createDefaultUI();
		addChildComponent(cui);
		extraParams.add(cui);
	}
}

bool GenericEasingUI::hitTest(int tx, int ty)
{
	bool result = EasingUI::hitTest(tx, ty);

	if (showFirstHandle && h1 != nullptr)
	{
		if (h1->getLocalBounds().contains(h1->getMouseXYRelative())) return true;
	}

	if (showLastHandle && h2 != nullptr)
	{
		if (h2->getLocalBounds().contains(h2->getMouseXYRelative())) return true;
	}

	for (auto& cui : extraParams)
	{
		Point<int> localPos = cui->getLocalPoint(this, Point<int>(tx, ty));
		if (cui->isVisible() && cui->hitTest(localPos.x, localPos.y)) return true;
	}

	return result;
}

void GenericEasingUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;

	if (h1 != nullptr)
	{
		Point<int> a = getUIPosForValuePos(easing->start + h1->parameter->getPoint());
		h1->setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(a));
	}

	if (h2 != nullptr)
	{
		Point<int> b = getUIPosForValuePos(easing->end + h2->parameter->getPoint());
		h2->setBounds(juce::Rectangle<int>(0, 0, 16, 16).withCentre(b));
	}

	juce::Rectangle<int> r = getLocalBounds().removeFromTop(20);
	for (auto& cui : extraParams) cui->setBounds(r.removeFromLeft(100).reduced(2));

	EasingUI::resized();
}


void GenericEasingUI::paintInternal(Graphics& g)
{

	if (!showFirstHandle && !showLastHandle) return;

	Point<int> p1 = Point<int>(getUIPosForValuePos(easing->start));
	Point<int> p2 = Point<int>(getUIPosForValuePos(easing->end));

	Colour c = LIGHTCONTOUR_COLOR;
	if (isMouseOver()) c = c.brighter();
	g.setColour(c);

	if (showFirstHandle && h1 != nullptr) g.drawLine(p1.x, p1.y, h1->getBounds().getCentreX(), h1->getBounds().getCentreY());
	if (showLastHandle && h2 != nullptr) g.drawLine(p2.x, p2.y, h2->getBounds().getCentreX(), h2->getBounds().getCentreY());

}

void GenericEasingUI::easingControllableFeedbackUpdate(Controllable* c)
{
	if ((h1 != nullptr && c == h1->parameter) || (h2 != nullptr && c ==  h2->parameter))
	{
		resized();
		shouldRepaint = true;
	}
}

void GenericEasingUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	EasingUI::setShowEasingHandles(showFirst, showLast);
	if(h1 != nullptr) h1->setVisible(showFirstHandle);
	if(h2 != nullptr) h2->setVisible(showLastHandle);

	for (auto& cui : extraParams) cui->setVisible(showFirst && showLast);

	resized();
	shouldRepaint = true;
}

void GenericEasingUI::mouseDown(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	EasingUI::mouseDown(e);


	if(h1 != nullptr) h1ValueAtMouseDown = h1->parameter->getPoint();
	if(h2 != nullptr) h2ValueAtMouseDown = h2->parameter->getPoint();
}

void GenericEasingUI::mouseDrag(const MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;

	if (e.eventComponent == h1.get() || e.eventComponent == h2.get())
	{
		Point2DParameter* targetAnchor = (e.eventComponent == h1.get()) ? h1->parameter : h2->parameter;
		Point<float> targetRefPoint = (e.eventComponent == h1.get()) ? easing->start : easing->end;

		Point<float> targetPoint = getValuePosForUIPos(e.getEventRelativeTo(this).getPosition()); //Point<float>(mp.x * 1.f / getWidth(), jmap<float>(mp.y, y1, y2, 0, 1));
		targetAnchor->setPoint(targetPoint - targetRefPoint);
	}
}

void GenericEasingUI::mouseUp(const MouseEvent& e)
{
	EasingUI::mouseUp(e);

	Array<UndoableAction*> actions;
	if(h1 != nullptr) actions.add(h1->parameter->setUndoablePoint(h1ValueAtMouseDown, h1->parameter->getPoint(), true));
	if(h2 != nullptr) actions.add(h2->parameter->setUndoablePoint(h2ValueAtMouseDown, h2->parameter->getPoint(), true));
	UndoMaster::getInstance()->performActions("Move anchors", actions);
}
