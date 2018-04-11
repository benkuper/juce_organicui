#include "TriggerButtonUI.h"
/*
  ==============================================================================

    TriggerButtonUI.cpp
    Created: 8 Mar 2016 3:45:53pm
    Author:  bkupe

  ==============================================================================
*/


//==============================================================================
TriggerButtonUI::TriggerButtonUI(Trigger *t) :
	TriggerUI(t),
	drawTriggering(false)
{
	setSize(20, 15);
	if (!forceFeedbackOnly) setRepaintsOnMouseActivity(true);
}

TriggerButtonUI::~TriggerButtonUI()
{
	stopTimer();
}

void TriggerButtonUI::triggerTriggered(const Trigger *)
{
	drawTriggering = true;
	repaint();
	startTimer(100);

}

void TriggerButtonUI::mouseDownInternal(const MouseEvent & e)
{
	if (forceFeedbackOnly) return;
	trigger->trigger();
}

bool TriggerButtonUI::hitTest(int x, int y)
{
	return drawRect.contains(x, y);
}

void TriggerButtonUI::paint (Graphics& g)
{
 juce::Rectangle<float> r = getLocalBounds().toFloat();
	if (!showLabel) r.setWidth(jmin<float>(r.getWidth(), 50));

    Point<float> center = r.getCentre();

	Colour c = BG_COLOR;
	if (!forceFeedbackOnly)
	{
		if (drawTriggering) c = HIGHLIGHT_COLOR;
		else c = isMouseOverOrDragging(true) ? (isMouseButtonDown() ? HIGHLIGHT_COLOR : NORMAL_COLOR.brighter()) : NORMAL_COLOR;
	}
	else
	{
		if (drawTriggering) c = FEEDBACK_COLOR;
		//DBG(trigger->niceName << " force feedback");
	}

    g.setGradientFill(ColourGradient(c,center.x,center.y,c.darker(.5f),2.f,2.f,true));
    g.fillRoundedRectangle(r.toFloat(), 4.f);
    g.setColour(c.darker());
    g.drawRoundedRectangle(r.toFloat(), 4.f, 2.f);

	if (showLabel)
	{
		g.setFont(10);
		g.setColour(Colours::white.darker(.1f));
		g.drawFittedText(trigger->niceName, getLocalBounds().reduced(2), Justification::centred, 1);
	}

	drawRect = r.toNearestInt();
}


void TriggerButtonUI::timerCallback()
{
	drawTriggering = false;
	repaint();
	stopTimer();
}
