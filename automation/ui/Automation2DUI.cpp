#include "../easing/Easing.h"
#include "Automation2DUI.h"

Automation2DUI::Automation2DUI(Automation* _automation) :
	BaseManagerViewUI(_automation->niceName, _automation),
	viewRange("View range", "Range to view around the position", 1, 0, _automation->length->floatValue()),
	numSteps(20),
	shouldRepaint(false)
{
	updatePositionOnDragMove = true;
	useCheckersAsUnits = true;
	centerUIAroundPosition = true;

	viewRangeUI.reset(viewRange.createTimeLabelParameter());
	manager->addAsyncContainerListener(this);

	addExistingItems();

	startTimerHz(40);
}

Automation2DUI::~Automation2DUI()
{
	if (!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
}

void Automation2DUI::paint(Graphics& g)
{
	BaseManagerViewUI::paint(g);

	if (itemsUI.size() <= 1) return;

	int totalSteps = numSteps * (itemsUI.size() - 1);
	float start = itemsUI[0]->item->position->floatValue();
	float end = itemsUI[itemsUI.size() - 1]->item->position->floatValue();
	float stepTime = (end - start) / totalSteps;

	//speed color feedback
	float minDist = .01f * stepTime;
	float maxDist = 2.f * stepTime;
	if (minDist == maxDist) return;

	Array<float> startValues = manager->getValuesForPosition(start);
	Point<float> prevPos(startValues[0], -startValues[1]);

	for (float t = start + stepTime; t <= end; t += stepTime)
	{
		Array<float> values = manager->getValuesForPosition(t);
		Point<float> curPos(values[0], -values[1]);

		float distMap = jlimit(0.f, 1.f, jmap(curPos.getDistanceFrom(prevPos), minDist, maxDist, 0.f, 1.f));
		g.setColour(BLUE_COLOR.withRotatedHue(distMap * .5f));
		g.drawLine(Line<int>(getPosInView(prevPos), getPosInView(curPos)).toFloat(), 1);

		prevPos.setXY(curPos.x, curPos.y);
	}

	for (auto & h : handles)
	{
		int index = manager->items.indexOf(h->key);
		if (index == manager->items.size() - 1) continue;

		AutomationKey2DUI* kui = h->isFirst ? itemsUI[index] : itemsUI[index + 1];

		g.setColour(NORMAL_COLOR);
		g.drawLine(kui->getBounds().getCentreX(), kui->getBounds().getCentreY(), h->getBounds().getCentreX(), h->getBounds().getCentreY(), 1);
	}

	Array<float> values = manager->getValuesForPosition(manager->position->floatValue());
	g.setColour(HIGHLIGHT_COLOR.withAlpha(.6f));
	g.drawEllipse(Rectangle<int>(0, 0, 10, 10).withCentre(getPosInView(Point<float>(values[0], -values[1]))).toFloat(), 2);

}

void Automation2DUI::resizedInternalContent(Rectangle<int>& r)
{
	BaseManagerViewUI::resizedInternalContent(r);
	placeHandles();
}

void Automation2DUI::updateViewUIPosition(AutomationKey2DUI* kui)
{
	BaseManagerViewUI::updateViewUIPosition(kui);
	placeHandles();
	shouldRepaint = true;
}

void Automation2DUI::placeHandles()
{
	for (auto& h : handles)
	{
		int index = manager->items.indexOf(h->key);
		if (index == manager->items.size() - 1) continue;

		AutomationKey* tk = h->isFirst ? manager->items[index] : manager->items[index + 1];

		CubicEasing* e0 = (CubicEasing*)h->key->easings[0];
		CubicEasing* e1 = (CubicEasing*)h->key->easings[1];

		Point<float> ep(tk->values[0]->floatValue() + (h->isFirst? e0->k1Anchor:e0->k2Anchor)->y, -(tk->values[1]->floatValue() + (h->isFirst ? e1->k1Anchor : e1->k2Anchor)->y));
		Point<int> vep = getPosInView(ep);

		h->setBounds(Rectangle<int>(vep.x - 5, vep.y - 5, 10, 10));
	}
}

void Automation2DUI::mouseDown(const MouseEvent& e)
{
	if (AutomationKey2DUI::Easing2DHandle* h = dynamic_cast<AutomationKey2DUI::Easing2DHandle*>(e.eventComponent))
	{
		//do nothing
	}
	else
	{
		BaseManagerViewUI::mouseDown(e);
	}
}

void Automation2DUI::mouseDrag(const MouseEvent& e)
{
	if (AutomationKey2DUI::Easing2DHandle* h = dynamic_cast<AutomationKey2DUI::Easing2DHandle*>(e.eventComponent))
	{
		int index = manager->items.indexOf(h->key);
		if (index == manager->items.size() - 1) return;

		AutomationKey* k = h->isFirst ? h->key : manager->items[index + 1];
		Point<float> p = getViewMousePosition();
		p -= k->viewUIPosition->getPoint();

		DBG(p.toString());
		CubicEasing * cex = dynamic_cast<CubicEasing*>(h->key->easings[0]);
		CubicEasing * cey = dynamic_cast<CubicEasing*>(h->key->easings[1]);

		Point2DParameter* kx = h->isFirst ? cex->k1Anchor : cex->k2Anchor;
		Point2DParameter* ky = h->isFirst ? cey->k1Anchor : cey->k2Anchor;

		kx->setPoint(kx->x, p.x);
		ky->setPoint(ky->x, -p.y);

		placeHandles();
	}
	else
	{
		BaseManagerViewUI::mouseDrag(e);
	}
}

void Automation2DUI::updateHandlesForKey(AutomationKey2DUI* kui)
{
	for (auto& h : kui->handles)
	{
		bool sel = kui->item->isSelected;
		if (!sel)
		{
			for (auto& e : kui->item->easings) if (e->isSelected) sel = true;
		}

		if (sel)
		{
			handles.add(h);
			addAndMakeVisible(h);
			h->addMouseListener(this, false);
		}
		else
		{
			handles.removeAllInstancesOf(h);
			removeChildComponent(h);
			h->removeMouseListener(this);
		}
	}
	placeHandles();
	repaint();
}

void Automation2DUI::addItemUIInternal(AutomationKey2DUI* kui)
{
	kui->addAsyncKey2DUIListener(this);
	updateHandlesForKey(kui);
}

void Automation2DUI::removeItemUIInternal(AutomationKey2DUI* kui)
{
	kui->removeAsyncKey2DUIListener(this); 
	for (auto& h : kui->handles)
	{
		handles.removeAllInstancesOf(h);
		removeChildComponent(h);
		h->removeMouseListener(this);
	}
}

void Automation2DUI::newMessage(const ContainerAsyncEvent& e)
{
	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
	{
		if (e.targetControllable == manager->position)
		{
			shouldRepaint = true;
		}
		else if (AutomationKey* k = e.targetControllable->getParentAs<AutomationKey>())
		{
			shouldRepaint = true;
		}
		else if (Easing* es = e.targetControllable->getParentAs<Easing>())
		{
			shouldRepaint = true;
		}
	}
	break;
	}
}

void Automation2DUI::newMessage(const AutomationKey2DUI::Key2DUIEvent& e)
{
	switch (e.type)
	{
	case AutomationKey2DUI::Key2DUIEvent::HANDLES_ADDED:
	{
		updateHandlesForKey(e.keyUI);
	}
	break;
	case AutomationKey2DUI::Key2DUIEvent::HANDLES_REMOVED:
	{
		for (auto& h : e.handles)
		{
			handles.removeAllInstancesOf(h);
			removeChildComponent(h);
			h->removeMouseListener(this);
		}

	}
	break;
	case AutomationKey2DUI::Key2DUIEvent::UI_SELECTION_CHANGED:
	{
		updateHandlesForKey(e.keyUI);
	}
	break;
	}

	repaint();
}

void Automation2DUI::timerCallback()
{
	if (shouldRepaint)
	{
		repaint();
		shouldRepaint = false;
	}
}
