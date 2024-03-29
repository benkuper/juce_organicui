#pragma once

class ParameterDetectiveWatcherUI :
	public ControllableDetectiveWatcherUI
{
public:
	ParameterDetectiveWatcherUI(ParameterDetectiveWatcher* watcher);
	~ParameterDetectiveWatcherUI();

	ParameterDetectiveWatcher* parameterWatcher;

	virtual void paintWatcherInternal(juce::Graphics& g, const juce::Rectangle<int>& r);
};
