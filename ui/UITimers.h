#pragma once

#define ORGANICUI_DEFAULT_TIMER 0
#define ORGANICUI_SLOW_TIMER 1


class UITimerTarget
{
public:
	UITimerTarget(int timerID = -1);
	virtual ~UITimerTarget();

	int paintTimerID;
	bool shouldRepaint;

	virtual void handlePaintTimer();
	virtual void handlePaintTimerInternal() = 0;


	juce::WeakReference<UITimerTarget>::Master masterReference;
	friend class juce::WeakReference<UITimerTarget>;
};

class OrganicUITimers :
	public juce::MultiTimer
{
public:
	juce_DeclareSingleton(OrganicUITimers, true);
	OrganicUITimers();
	~OrganicUITimers() {}

	juce::HashMap<int, juce::Array<juce::WeakReference<UITimerTarget>>> timerMap;

	void registerTarget(int timerID, UITimerTarget* ui);
	void unregisterTarget(int timerID, UITimerTarget* ui);
	void timerCallback(int timerID);
};