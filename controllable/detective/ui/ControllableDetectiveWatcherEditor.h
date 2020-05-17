#pragma once

class ControllableDetectiveWatcherEditor :
	public BaseItemEditor,
	public Timer
{
public:
	ControllableDetectiveWatcherEditor(ControllableDetectiveWatcher * watcher, bool isRoot);
	~ControllableDetectiveWatcherEditor();

	ControllableDetectiveWatcher* controllableWatcher;

	juce::Rectangle<int> canvasRect;
	std::unique_ptr<TimeLabel> watchTimeUI;

	virtual void setCollapsed(bool value, bool force = false, bool animate = true, bool doNotRebuild = false) override;

	virtual void paint(Graphics &g) override;
	virtual void paintWatcherInternal(Graphics& g, const juce::Rectangle<int>& r) {}

	virtual void resizedInternalHeaderItemInternal(juce::Rectangle<int>& r) override;
	virtual void resizedInternalContent(juce::Rectangle<int>& r) override;

	void timerCallback() override;
};