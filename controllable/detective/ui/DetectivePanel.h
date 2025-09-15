#pragma once


class DetectivePanel :
	public ManagerShapeShifterUI<Detective,ControllableDetectiveWatcher>
{
public:
	DetectivePanel(const juce::String& name);
	~DetectivePanel();

	static DetectivePanel* create(const juce::String& contentName) { return new DetectivePanel(contentName); }
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetectivePanel)
};