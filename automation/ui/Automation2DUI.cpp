#include "Automation2DUI.h"
#pragma once


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
	if(!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
}

void Automation2DUI::paint(Graphics& g)
{
	BaseManagerViewUI::paint(g);
	
	if (itemsUI.size() <= 1) return;

	int totalSteps = numSteps * (itemsUI.size()-1);
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
		g.setColour(BLUE_COLOR.withRotatedHue(distMap*.5f));
		g.drawLine(Line<int>(getPosInView(prevPos), getPosInView(curPos)).toFloat(), 1);

		prevPos.setXY(curPos.x, curPos.y);
	}

	Array<float> values = manager->getValuesForPosition(manager->position->floatValue());
	g.setColour(HIGHLIGHT_COLOR.withAlpha(.6f));
	g.drawEllipse(Rectangle<int>(0, 0, 10,10).withCentre(getPosInView(Point<float>(values[0], -values[1]))).toFloat(), 2);

}

void Automation2DUI::updateViewUIPosition(AutomationKey2DUI* kui)
{
	BaseManagerViewUI::updateViewUIPosition(kui);
	shouldRepaint = true;
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

void Automation2DUI::timerCallback()
{
	if (shouldRepaint)
	{
		repaint();
		shouldRepaint = false;
	}
}
