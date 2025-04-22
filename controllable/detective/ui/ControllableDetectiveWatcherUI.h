#pragma once

class ControllableDetectiveWatcherUI :
	public ItemUI<ControllableDetectiveWatcher>,
	public juce::Timer
{
public:
	ControllableDetectiveWatcherUI(ControllableDetectiveWatcher* watcher);
	virtual ~ControllableDetectiveWatcherUI();

	juce::Rectangle<int> canvasRect;
	TargetParameterUI targetUI;
	TimeLabel watchTimeUI;
	juce::Image canvasSnapshot;
	bool snapshotMode;

	virtual void paint(juce::Graphics& g) override;
	virtual void paintWatcherInternal(juce::Graphics& g, const juce::Rectangle<int>& r) {}

	virtual void resizedInternalHeader(juce::Rectangle<int>& r) override;
	virtual void resizedInternalContent(juce::Rectangle<int>& r) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;

	void timerCallback() override;
};