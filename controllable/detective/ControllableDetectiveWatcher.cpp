ControllableDetectiveWatcher::ControllableDetectiveWatcher() :
	BaseItem("No target"),
	Thread("Watcher"),
	controllable(nullptr),
	oldestVal(0)
{
	isSelectable = false;

	userCanDuplicate = false;

	target = addTargetParameter("Target", "Target to watch");
	target->excludeTypesFilter.add(Trigger::getTypeStringStatic());
	target->hideInEditor = true;

	watchTime = addFloatParameter("Time Window", "The time window to watch", 3, .1f);
	watchTime->defaultUI = FloatParameter::TIME;
	watchTime->hideInEditor = true;
}

ControllableDetectiveWatcher::~ControllableDetectiveWatcher()
{
	stopThread(100);
}

void ControllableDetectiveWatcher::clearItem()
{
	setControllable(nullptr);
}

void ControllableDetectiveWatcher::setControllable(Controllable* c)
{
	if (c == controllable) return;

	if (controllable != nullptr)
	{
		stopThread(100);
	}
	
	controllable = c;

	if (controllable != nullptr)
	{
		stopThread(100);
	}

	data.clear();
	setNiceName(controllable != nullptr ? controllable->niceName : "No target");
}

void ControllableDetectiveWatcher::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == enabled)
	{
		if (enabled->boolValue()) startThread();
		else
		{
			stopThread(100);
		}
	}
	else if (p == target)
	{
		setControllable(target->target);
	}
}

void ControllableDetectiveWatcher::addValue(var val)
{
	data.add(new WatcherData(Time::getMillisecondCounter() / 1000.0f, val));
}

void ControllableDetectiveWatcher::run()
{
	while (!threadShouldExit() && !controllable.wasObjectDeleted())
	{
		float curTime = Time::getMillisecondCounter() / 1000.0f;
		float maxTime = curTime - watchTime->floatValue();

		bool foundInRange = false;
		for (int i = 0; i < data.size(); ++i)
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
