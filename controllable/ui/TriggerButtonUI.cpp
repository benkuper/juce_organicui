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
	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);
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
	if (!showLabel) r.setWidth(jmin<float>(r.getWidth(), r.getHeight()*3));

	Point<float> center = r.getCentre();

	Colour bgColor = useCustomBGColor ? customBGColor : NORMAL_COLOR;
	Colour c = bgColor.darker();

	if (isInteractable())
	{
		if (drawTriggering) c = HIGHLIGHT_COLOR;
		else c = isMouseOverOrDragging(true) ? (isMouseButtonDown() ? HIGHLIGHT_COLOR : bgColor.brighter()) : bgColor;
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
		g.setFont(jlimit(10, 40, jmin(tr.getHeight()-4,tr.getWidth()) - 16));
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
