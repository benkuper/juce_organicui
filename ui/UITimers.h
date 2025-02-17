#pragma once

#define ORGANICUI_DEFAULT_TIMER 0
#define ORGANICUI_SLOW_TIMER 1

#define ORGANICUI_FPS_TIMER 99

//#define ORGANICUI_LOG_FPS_DEBUG


class UITimerTarget
{
public:
	UITimerTarget(int timerID = -1, juce::String _name = "");
	virtual ~UITimerTarget();

	int paintTimerID;
	bool shouldRepaint;
	bool paintingAsked;

	juce::String name;

	virtual void handlePaintTimer();
	virtual void handlePaintTimerInternal() = 0;
	virtual void validatePaint();


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
	juce::HashMap<int, juce::uint32> lastRepaintTimes;
#ifdef ORGANICUI_LOG_FPS_DEBUG
	juce::HashMap<int, int> fps;
#endif

	void registerTarget(int timerID, UITimerTarget* ui);
	void unregisterTarget(int timerID, UITimerTarget* ui);
	void timerCallback(int timerID);
};