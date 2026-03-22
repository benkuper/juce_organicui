/*
==============================================================================

TriggerBlinkUI.cpp
Created: 8 Mar 2016 3:45:53pm
Author:  bkupe

==============================================================================
*/

//==============================================================================
TriggerBlinkUI::TriggerBlinkUI(Array<Trigger*> triggers) :
	TriggerUI(triggers),
	intensity(0),
	animateIntensity(true),
	blinkTime(100),
	offColor(NORMAL_COLOR),
	onColor(FEEDBACK_COLOR),
	refreshPeriod(50)
{
	setSize(30, 20);

}

TriggerBlinkUI::~TriggerBlinkUI()
{

}

void TriggerBlinkUI::setTriggerReference(Trigger* t) {
	if (trigger != nullptr) {
		trigger->removeAsyncTriggerListener(this);
	}

	trigger = t;
	trigger->addAsyncTriggerListener(this);
}

void TriggerBlinkUI::triggerTriggered(const Trigger*) {
	startBlink();
}

void TriggerBlinkUI::paint(Graphics& g)
{
	if (!trigger.get())return;

	Colour curOffColor = useCustomBGColor ? customBGColor : offColor;

	g.setColour(curOffColor.interpolatedWith(onColor, intensity));
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 2);
	if (showLabel)
	{
		juce::Rectangle<int> tr = getLocalBounds().reduced(2);
		g.setFont(jlimit(12, 40, jmin(tr.getHeight(), tr.getWidth()) - 16));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : trigger->niceName, tr, Justification::centred, 1);
	}
}


void TriggerBlinkUI::startBlink() {
	intensity = 1;
	if (!animateIntensity) repaint();
	startTimer(animateIntensity ? refreshPeriod : blinkTime);
}

void TriggerBlinkUI::timerCallback() {

	if (animateIntensity)
	{
		intensity -= refreshPeriod * 1.0f / blinkTime;

		if (intensity < 0) {
			intensity = 0;
			stopTimer();
		}
	}
	else
	{
		intensity = 0;
		stopTimer();
	}

	repaint();
}
