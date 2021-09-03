#pragma once

class ControllableDetectiveWatcherUI;

class ControllableDetectiveWatcher :
	public BaseItem,
	public Thread
{
public:
	ControllableDetectiveWatcher();
	virtual ~ControllableDetectiveWatcher();

	virtual void clearItem() override;

	WeakReference<Controllable> controllable;

	TargetParameter* target;
	FloatParameter* watchTime;

	struct WatcherData
	{
		WatcherData(double time, var val) : time(time), val(val) {}
		double time;
		var val;
	};

	OwnedArray<WatcherData, CriticalSection> data;
	var oldestVal;

	virtual void setControllable(Controllable* c);

	void onContainerParameterChangedInternal(Parameter* p) override;

	void addValue(var data);
	void run() override;

	virtual ControllableDetectiveWatcherUI* getUI();
};