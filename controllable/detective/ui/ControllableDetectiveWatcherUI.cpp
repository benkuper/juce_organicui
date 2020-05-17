
ControllableDetectiveWatcherUI::ControllableDetectiveWatcherUI(ControllableDetectiveWatcher* watcher) :
	BaseItemUI(watcher, Direction::VERTICAL),
	watchTimeUI(watcher->watchTime),
	snapshotMode(false)
{
	watchTimeUI.setOpaqueBackground(false);
	addAndMakeVisible(&watchTimeUI);

	if (item->enabled->boolValue()) startTimerHz(20);
}

ControllableDetectiveWatcherUI::~ControllableDetectiveWatcherUI()
{

}

void ControllableDetectiveWatcherUI::paint(Graphics& g)
{
	BaseItemUI::paint(g);
	if (item->editorIsCollapsed) return;
	g.setColour(BG_COLOR);
	g.fillRect(canvasRect);
	if (item->enabled->boolValue() || snapshotMode) paintWatcherInternal(g, canvasRect);
	else g.drawImage(canvasSnapshot, canvasRect.toFloat());
}

void ControllableDetectiveWatcherUI::resizedInternalHeader(juce::Rectangle<int>& r)
{
	watchTimeUI.setBounds(r.removeFromRight(100).reduced(2));
}

void ControllableDetectiveWatcherUI::resizedInternalContent(juce::Rectangle<int>& r)
{
	canvasRect = r.reduced(2);
}

void ControllableDetectiveWatcherUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemUI::controllableFeedbackUpdateInternal(c);
	if (c == item->enabled)
	{
		if (item->enabled->boolValue())
		{
			startTimerHz(20);
		}
		else
		{
			stopTimer();
			snapshotMode = true;
			canvasSnapshot = createComponentSnapshot(canvasRect);
			snapshotMode = false;
		}
	}
}

void ControllableDetectiveWatcherUI::timerCallback()
{
	repaint();
}
