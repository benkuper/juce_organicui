/*
  ==============================================================================

    TriggerImageUI.cpp
    Created: 4 Jan 2017 1:32:47pm
    Author:  Ben

  ==============================================================================
*/

TriggerImageUI::TriggerImageUI(Array<Trigger *> t, const Image &i, bool keepSaturation) :
	TriggerUI(t),
	onImage(i),
	offImage(i.createCopy()),
	drawTriggering(false),
	forceDrawTriggering(false)
{
	showLabel = false;

	if(!keepSaturation) offImage.desaturate();
	if(trigger->isControllableFeedbackOnly) offImage.multiplyAllAlphas(.5f);
	setRepaintsOnMouseActivity(true);
}

TriggerImageUI::~TriggerImageUI()
{
	stopTimer();
}

void TriggerImageUI::paint(Graphics & g)
{
	if (getWidth() == 0 || getHeight() == 0) return;

	bool drawT = drawTriggering || forceDrawTriggering;
	g.drawImage(drawT?onImage:offImage, getLocalBounds().toFloat());
	
	if (!drawT)
	{
		g.setColour(Colours::black.withAlpha(.1f));
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 4);
	}

	/*
	if (isMouseOver() && !trigger->isControllableFeedbackOnly)
	{
		g.setColour(HIGHLIGHT_COLOR.withAlpha(.2f));
		g.fillEllipse(getLocalBounds().reduced(2).toFloat());
	}
	*/

	if (showLabel)
	{
		juce::Rectangle<int> tr = getLocalBounds().reduced(2);
		float fontSize = customTextSize > 0 ? customTextSize : jlimit(10, 40, jmin(tr.getHeight() - 4, tr.getWidth()) - 16);
		g.setFont(fontSize);
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : trigger->niceName, tr, Justification::centred, 1);
	}
}

void TriggerImageUI::triggerTriggered(const Trigger *)
{
	if (!drawTriggering)
	{
		drawTriggering = true;
		repaint();
	}

	startTimer(100);
}

void TriggerImageUI::mouseDownInternal(const MouseEvent &)
{
	if (controllable->isControllableFeedbackOnly) return;
	trigger->trigger();
}

void TriggerImageUI::timerCallback()
{
	drawTriggering = false;
	repaint();
	stopTimer();
}
