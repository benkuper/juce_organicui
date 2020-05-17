#pragma once

class ControllableDetectiveWatcherUI :
	public BaseItemUI<ControllableDetectiveWatcher>,
	public Timer
{
public:
	ControllableDetectiveWatcherUI(ControllableDetectiveWatcher* watcher);
	virtual ~ControllableDetectiveWatcherUI();

	juce::Rectangle<int> canvasRect;
	TargetParameterUI targetUI;
	TimeLabel watchTimeUI;
	Image canvasSnapshot;
	bool snapshotMode;

	virtual void paint(Graphics& g) override;
	virtual void paintWatcherInternal(Graphics& g, const juce::Rectangle<int>& r) {}

	virtual void resizedInternalHeader(juce::Rectangle<int>& r) override;
	virtual void resizedInternalContent(juce::Rectangle<int>& r) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;

	void timerCallback() override;
};