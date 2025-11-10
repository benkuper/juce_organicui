
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
	OwnedArray<Path> paths;
	int numParamValues = parameterWatcher->parameter->isComplex() ? parameterWatcher->parameter->value.size() : 1;

	{
		GenericScopedLock<CriticalSection> lock(parameterWatcher->data.getLock());

		float curTime = Time::getMillisecondCounter() / 1000.0f;
		float watchWindow = parameterWatcher->watchTime->floatValue();

		for (int pi = 0; pi < numParamValues; ++pi)
		{
			float minV = INT32_MAX;
			float maxV = INT32_MIN;

			if (!p->hasRange())
			{
				for (int i = parameterWatcher->data.size() - 1; i >= 0; i--)
				{
					ControllableDetectiveWatcher::WatcherData* d = parameterWatcher->data[i];
					float val = numParamValues > 1 ? d->val[pi] : d->val;
					minV = jmin<float>(minV, val);
					maxV = jmax<float>(maxV, val);
				}
			}
			else
			{
				minV = (float)(numParamValues > 1 ? p->minimumValue[pi] : p->minimumValue);
				maxV = (float)(numParamValues > 1 ? p->maximumValue[pi] : p->maximumValue);
			}

			if (minV == maxV) return;

			Path* path = new Path();
			paths.add(path);

			var firstVal = parameterWatcher->data.size() > 0 ? parameterWatcher->data[parameterWatcher->data.size() - 1]->val : parameterWatcher->oldestVal;

			int sy = jmap<float>(numParamValues > 1 ? firstVal[pi] : firstVal, minV, maxV, r.getBottom(), r.getY());
			path->startNewSubPath(r.getRight(), sy);

			int prevX = r.getRight();// , prevY = sy;
			for (int i = parameterWatcher->data.size() - 1; i >= 0; i--)
			{
				ControllableDetectiveWatcher::WatcherData* d = parameterWatcher->data[i];
				int tx = r.getX() + (1 - ((curTime - d->time) / watchWindow)) * r.getWidth();
				if (tx == prevX) continue;

				int ty = jmap<float>(numParamValues > 1 ? d->val[pi] : d->val, minV, maxV, r.getBottom(), r.getY());
				if (prevX - tx > 1) path->lineTo(prevX, ty);
				path->lineTo(tx, ty);
				prevX = tx;
			}

			float lastY = jmap<float>(numParamValues > 1 ? item->oldestVal[pi] : item->oldestVal, minV, maxV, r.getBottom(), r.getY());
			path->lineTo(prevX, lastY);
			path->lineTo(r.getX(), lastY);
		}
	}

	for (int i = 0; i < paths.size(); ++i)
	{
		if (numParamValues > 1) g.setColour(Colour::fromHSV(.03f + .3f * i, 1.f, 1.f, 1.f));
		else g.setColour(FRONT_COLOR);
		g.strokePath(*paths[i], PathStrokeType(.5f));
	}
}
