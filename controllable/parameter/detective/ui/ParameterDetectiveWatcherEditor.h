#pragma once

class ParameterDetectiveWatcherEditor :
	public ControllableDetectiveWatcherEditor
{
public:
	ParameterDetectiveWatcherEditor(ParameterDetectiveWatcher* watcher, bool isRoot);
	~ParameterDetectiveWatcherEditor();

	ParameterDetectiveWatcher* parameterWatcher;

	virtual void paintWatcherInternal(Graphics& g, const juce::Rectangle<int>& r);
};
