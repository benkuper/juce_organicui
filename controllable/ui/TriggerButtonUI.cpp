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
	setRepaintsOnMouseActivity(isInteractable());
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
	trigger->trigger();
}

void TriggerButtonUI::paint(Graphics& g)
{
	juce::Rectangle<float> r = getLocalBounds().toFloat();
	if (!showLabel) r.setWidth(jmin<float>(r.getWidth(), 50));

	Point<float> center = r.getCentre();

	Colour c = BG_COLOR;

	if (isInteractable())
	{
		if (drawTriggering) c = HIGHLIGHT_COLOR;
		else c = isMouseOverOrDragging(true) ? (isMouseButtonDown() ? HIGHLIGHT_COLOR : NORMAL_COLOR.brighter()) : NORMAL_COLOR;
	}
	else
	{
		if (drawTriggering) c = FEEDBACK_COLOR;
		//DBG(trigger->niceName << " force feedback");
	}

	g.setGradientFill(ColourGradient(c, center.x, center.y, c.darker(.5f), 2.f, 2.f, true));
	g.fillRoundedRectangle(r.toFloat(), 4.f);
	g.setColour(c.darker());
	g.drawRoundedRectangle(r.toFloat(), 4.f, 2.f);

	if (showLabel)
	{
		Rectangle<int> tr = getLocalBounds().reduced(2);
		g.setFont(jlimit(12, 40, jmin(tr.getHeight(),tr.getWidth()) - 16));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : trigger->niceName, tr, Justification::centred, 1);
	}
}


void TriggerButtonUI::timerCallback()
{
	drawTriggering = false;
	repaint();
	stopTimer();
}
