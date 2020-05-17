
ControllableDetectiveWatcher::ControllableDetectiveWatcher(Controllable* p) :
	BaseItem(p->niceName),
	Thread("Watcher"),
	controllable(p),
	oldestVal(0)
{
	isSelectable = false;

	userCanDuplicate = false;

	watchTime = addFloatParameter("Time Window", "The time window to watch", 3, .1f, 3600);
	watchTime->defaultUI = FloatParameter::TIME;
	watchTime->hideInEditor = true;

	//fps = addFloatParameter("FPS", "The number of samples per second to record. Keep 0 to record everything", 0, 0, 100);

	startThread();
}

ControllableDetectiveWatcher::~ControllableDetectiveWatcher()
{
	signalThreadShouldExit();
	waitForThreadToExit(100);
}

void ControllableDetectiveWatcher::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == enabled)
	{
		if (enabled->boolValue()) startThread();
		else
		{
			signalThreadShouldExit();
			waitForThreadToExit(100);
		}
	}
}

void ControllableDetectiveWatcher::addValue(var val)
{
	data.add(new WatcherData(Time::getMillisecondCounter() / 1000.0f, val));
}

void ControllableDetectiveWatcher::run()
{
	while (!threadShouldExit())
	{
		float curTime = Time::getMillisecondCounter() / 1000.0f;
		float maxTime = curTime - watchTime->floatValue();

		bool foundInRange = false;
		for (int i = 0; i < data.size(); i++)
		{
			if (data[i]->time > maxTime)
			{
				if(i > 0) oldestVal = data[i-1]->val.clone();
				data.removeRange(0, i);
				foundInRange = true;
				break;
			}
		}

		if (!foundInRange)
		{
			if (data.size() > 0) oldestVal = data[data.size() - 1]->val.clone();
			data.clear();
		}

		sleep(50);
	}
	
}

ControllableDetectiveWatcherUI* ControllableDetectiveWatcher::getUI()
{
	return new ControllableDetectiveWatcherUI(this);
}
