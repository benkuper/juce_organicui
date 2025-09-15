#pragma once

class ControllableDetectiveWatcherUI;

class ControllableDetectiveWatcher :
	public BaseItem,
	public juce::Thread
{
public:
	ControllableDetectiveWatcher();
	virtual ~ControllableDetectiveWatcher();

	virtual void clearItem() override;

	juce::WeakReference<Controllable> controllable;

	TargetParameter* target;
	FloatParameter* watchTime;

	struct WatcherData
	{
		WatcherData(double time, juce::var val) : time(time), val(val) {}
		double time;
		juce::var val;
	};

	juce::OwnedArray<WatcherData, juce::CriticalSection> data;
	juce::var oldestVal;

	virtual void setControllable(Controllable* c);

	void onContainerParameterChangedInternal(Parameter* p) override;

	void addValue(juce::var data);
	void run() override;

	virtual BaseItemMinimalUI* createUI() override;
};