#pragma once


class DetectivePanel :
	public BaseManagerShapeShifterUI<Detective,ControllableDetectiveWatcher,ControllableDetectiveWatcherUI>
{
public:
	DetectivePanel(const String& name);
	~DetectivePanel();

	ControllableDetectiveWatcherUI* createUIForItem(ControllableDetectiveWatcher* w) override;

	static DetectivePanel* create(const String& contentName) { return new DetectivePanel(contentName); }
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetectivePanel)
};