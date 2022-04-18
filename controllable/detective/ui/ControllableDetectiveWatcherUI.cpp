
ControllableDetectiveWatcherUI::ControllableDetectiveWatcherUI(ControllableDetectiveWatcher* watcher) :
	BaseItemUI(watcher, Direction::VERTICAL),
	targetUI(watcher->target),
	watchTimeUI(watcher->watchTime),
	snapshotMode(false)
{
	watchTimeUI.setOpaqueBackground(false);
	addAndMakeVisible(&watchTimeUI);

	addAndMakeVisible(&targetUI);

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

	if (item->controllable == nullptr || item->controllable.wasObjectDeleted())
	{
		g.setColour(TEXT_COLOR);
		g.drawText("Target destroyed", canvasRect,Justification::centred);
		return;
	}

	if (item->enabled->boolValue() || snapshotMode) paintWatcherInternal(g, canvasRect);
	else g.drawImage(canvasSnapshot, canvasRect.toFloat());
}

void ControllableDetectiveWatcherUI::resizedInternalHeader(juce::Rectangle<int>& r)
{
	watchTimeUI.setBounds(r.removeFromRight(200).reduced(2));
	targetUI.setBounds(r.removeFromRight(160).reduced(1));
}

void ControllableDetectiveWatcherUI::resizedInternalContent(juce::Rectangle<int>& r)
{
	canvasRect = r.reduced(2);
}

void ControllableDetectiveWatcherUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemUI::controllableFeedbackUpdateInternal(c);
	if (c == item->enabled || c == item->target)
	{
		if (item->enabled->boolValue() && (item->controllable != nullptr && !item->controllable.wasObjectDeleted()))
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

		repaint();
	}
}

void ControllableDetectiveWatcherUI::timerCallback()
{
	repaint();
}
