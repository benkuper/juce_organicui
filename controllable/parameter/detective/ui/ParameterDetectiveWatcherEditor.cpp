
ParameterDetectiveWatcherEditor::ParameterDetectiveWatcherEditor(ParameterDetectiveWatcher* watcher, bool isRoot) :
	ControllableDetectiveWatcherEditor(watcher, isRoot),
	parameterWatcher(watcher)
{

}

ParameterDetectiveWatcherEditor::~ParameterDetectiveWatcherEditor()
{
}

void ParameterDetectiveWatcherEditor::paintWatcherInternal(Graphics& g, const juce::Rectangle<int>& r)
{
	if (parameterWatcher->data.size() == 0) return;

	Parameter* p = parameterWatcher->parameter.get();
	parameterWatcher->data.getLock().enter();

	float curTime = Time::getMillisecondCounter() / 1000.0f;
	float watchWindow = parameterWatcher->watchTime->floatValue();
	float minV = p->hasRange() ? p->minimumValue : 0;
	float maxV = p->hasRange() ? p->maximumValue : 1;

	Path path;
	path.startNewSubPath(r.getRight(), jmap<float>(parameterWatcher->data[parameterWatcher->data.size() - 1]->val, minV, maxV, r.getBottom(), r.getY()));

	for (int i = parameterWatcher->data.size() - 1; i >= 0; i--)
	{
		ControllableDetectiveWatcher::WatcherData * d =  parameterWatcher->data[i];
		int tx = r.getX() + (1 - ((curTime - d->time) / watchWindow)) * r.getWidth();
		int ty = jmap<float>(d->val, minV, maxV, r.getBottom(), r.getY());
		path.lineTo(tx, ty);
	}

	parameterWatcher->data.getLock().exit();

	g.setColour(NORMAL_COLOR);
	g.strokePath(path, PathStrokeType(1));
	
}
