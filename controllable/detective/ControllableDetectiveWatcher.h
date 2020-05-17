#pragma once

class ControllableDetectiveWatcher :
	public BaseItem,
	public Thread
{
public:
	ControllableDetectiveWatcher(Controllable * p = nullptr);
	~ControllableDetectiveWatcher();

	WeakReference<Controllable> controllable;

	FloatParameter* watchTime;
	//IntParameter* fps;

	struct WatcherData
	{
		WatcherData(float time, var val) : time(time), val(val) {}
		float time;
		var val;
	};

	OwnedArray<WatcherData, CriticalSection> data;

	void addValue(var data);

	void run() override;

	InspectableEditor* getEditor(bool isRoot) override;

};