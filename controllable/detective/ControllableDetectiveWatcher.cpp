
ControllableDetectiveWatcher::ControllableDetectiveWatcher(Controllable* p) :
	BaseItem(p->niceName),
	Thread("Watcher"),
	controllable(p)
{
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

		for (int i = 0; i < data.size(); i++)
		{
			if (data[i]->time > maxTime)
			{
				data.removeRange(0, i);
				DBG("Remove range : " << i << ", new data size : " << data.size());
				break;
			}
		}

		sleep(10);
	}
	
}

InspectableEditor* ControllableDetectiveWatcher::getEditor(bool isRoot)
{
	return new ControllableDetectiveWatcherEditor(this, isRoot);
}
