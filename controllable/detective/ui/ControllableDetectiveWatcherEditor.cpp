
ControllableDetectiveWatcherEditor::ControllableDetectiveWatcherEditor(ControllableDetectiveWatcher* watcher, bool isRoot) :
	BaseItemEditor(watcher, isRoot),
	controllableWatcher(watcher)
{
	watchTimeUI.reset(controllableWatcher->watchTime->createTimeLabelParameter());
	addAndMakeVisible(watchTimeUI.get());

	if (!item->editorIsCollapsed) startTimerHz(20);
}

ControllableDetectiveWatcherEditor::~ControllableDetectiveWatcherEditor()
{

}

void ControllableDetectiveWatcherEditor::setCollapsed(bool value, bool force, bool animate, bool doNotRebuild)
{
	BaseItemEditor::setCollapsed(value, force, animate, doNotRebuild);
	if (item->editorIsCollapsed) stopTimer();
	else startTimerHz(20);
}

void ControllableDetectiveWatcherEditor::paint(Graphics& g)
{
	BaseItemEditor::paint(g);
	if (item->editorIsCollapsed) return;
	g.setColour(BG_COLOR.darker());
	g.fillRect(canvasRect);
	paintWatcherInternal(g, canvasRect);
}

void ControllableDetectiveWatcherEditor::resizedInternalHeaderItemInternal(juce::Rectangle<int>& r)
{
	watchTimeUI->setBounds(r.removeFromRight(100));
}

void ControllableDetectiveWatcherEditor::resizedInternalContent(juce::Rectangle<int>& r)
{
	canvasRect = r.withHeight(100).reduced(2);
	r.translate(0,100);
}

void ControllableDetectiveWatcherEditor::timerCallback()
{
	repaint();
}
