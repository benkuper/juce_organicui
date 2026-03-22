/*
  ==============================================================================

	TriggerButtonUI.cpp
	Created: 8 Mar 2016 3:45:53pm
	Author:  bkupe

  ==============================================================================
*/


//==============================================================================
TriggerButtonUI::TriggerButtonUI(Array<Trigger *> triggers) :
    TriggerUI(triggers),
	drawTriggering(false)
{
	setSize(Font(12).getStringWidth(triggers[0]->niceName)+4, GlobalSettings::getInstance()->fontSize->floatValue() + 4);
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
	if (trigger.wasObjectDeleted()) return;

	for (auto& t : triggers)
	{
		if (t == nullptr) continue;
		t->trigger();
	}
}

bool TriggerButtonUI::hitTest(int x, int y)
{
	if (!TriggerUI::hitTest(x, y)) return false;
	return hitRect.contains(x, y);
}

void TriggerButtonUI::paint(Graphics& g)
{
	if (trigger == nullptr) return;

	hitRect = getLocalBounds().toFloat();
	if (!showLabel) hitRect.setWidth(jmin<float>(hitRect.getWidth(), hitRect.getHeight()*3));

	Point<float> center = hitRect.getCentre();

	Colour bgColor = useCustomBGColor ? customBGColor : NORMAL_COLOR;
	Colour c = bgColor.darker();

	if (trigger->isControlledByParrot) c = Colours::rebeccapurple.brighter(.2f);

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
	g.fillRoundedRectangle(hitRect.toFloat(), 4.f);
	g.setColour(c.darker());
	g.drawRoundedRectangle(hitRect.toFloat(), 4.f, 2.f);

	if (showLabel)
	{
		juce::Rectangle<int> tr = getLocalBounds().reduced(2);
		float fontSize = customTextSize > 0 ? customTextSize : jlimit(10, 40, jmin(tr.getHeight() - 4, tr.getWidth()) - 16);
		g.setFont(fontSize);
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