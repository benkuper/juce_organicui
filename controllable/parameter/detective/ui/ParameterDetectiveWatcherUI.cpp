
ParameterDetectiveWatcherUI::ParameterDetectiveWatcherUI(ParameterDetectiveWatcher* watcher) :
	ControllableDetectiveWatcherUI(watcher),
	parameterWatcher(watcher)
{

}

ParameterDetectiveWatcherUI::~ParameterDetectiveWatcherUI()
{
}

void ParameterDetectiveWatcherUI::paintWatcherInternal(Graphics& g, const juce::Rectangle<int>& r)
{

	Parameter* p = parameterWatcher->parameter.get();
	parameterWatcher->data.getLock().enter();

	float curTime = Time::getMillisecondCounter() / 1000.0f;
	float watchWindow = parameterWatcher->watchTime->floatValue();
	
	OwnedArray<Path> paths;
	
	int numParamValues = parameterWatcher->parameter->isComplex() ? parameterWatcher->parameter->value.size() : 1;
	for (int pi = 0; pi < numParamValues; pi++)
	{
		float minV = p->hasRange() ? (numParamValues > 1 ? p->minimumValue[pi]: p->minimumValue) : 0;
		float maxV = p->hasRange() ? (numParamValues > 1 ? p->maximumValue[pi] : p->maximumValue) : 1;
		
		Path* path = new Path();
		paths.add(path);

		var firstVal = parameterWatcher->data.size() > 0 ? parameterWatcher->data[parameterWatcher->data.size() - 1]->val : parameterWatcher->oldestVal;

		int sy = jmap<float>(numParamValues > 1?firstVal[pi]:firstVal, minV, maxV, r.getBottom(), r.getY());
		path->startNewSubPath(r.getRight(), sy);

		int prevX = r.getRight();// , prevY = sy;
		for (int i = parameterWatcher->data.size() - 1; i >= 0; i--)
		{
			ControllableDetectiveWatcher::WatcherData* d = parameterWatcher->data[i];
			int tx = r.getX() + (1 - ((curTime - d->time) / watchWindow)) * r.getWidth();
			if (tx == prevX) continue;

			int ty = jmap<float>(numParamValues > 1?d->val[pi]:d->val, minV, maxV, r.getBottom(), r.getY());
			if (prevX - tx > 1) path->lineTo(prevX, ty);
			path->lineTo(tx, ty);
			prevX = tx;
		}

		float lastY = jmap<float>(numParamValues > 1?item->oldestVal[pi]:item->oldestVal, minV, maxV, r.getBottom(), r.getY());
		path->lineTo(prevX, lastY);
		path->lineTo(r.getX(), lastY);
	}
	
	parameterWatcher->data.getLock().exit();

	for (int i=0;i<paths.size();i++)
	{
		if (numParamValues > 1) g.setColour(Colour::fromHSV(.03f + .3f * i, 1, 1, 1));
		else g.setColour(FRONT_COLOR);
		g.strokePath(*paths[i], PathStrokeType(.5f));
	}
}
