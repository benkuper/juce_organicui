/*
  ==============================================================================

    Automation.h
    Created: 11 Dec 2016 1:21:37pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class InspectableSelectionManager;


class Automation :
	public BaseManager<AutomationKey>
{
public:
	Automation(const String &name = "Automation", int numDimensions = 1, AutomationRecorder * recorder = nullptr, bool allowKeysOutside = false, bool dedicatedSelectionManager = true);
	virtual ~Automation();

	int numDimensions;

	//Recorder
	AutomationRecorder * recorder;

	//ui
	bool showUIInEditor;

	//Position and value
	bool hasRange;
	Array<float> minimumValues;
	Array<float> maximumValues;

	bool allowKeysOutside; //allow keys positions to be outside automation timing
	FloatParameter * position;
	Array<FloatParameter *> values;
	FloatParameter * length;

	//snapping
	Array<float> snapPositions;
	BoolParameter * enableSnap;
	FloatParameter * snapSensitivity;

	std::unique_ptr<InspectableSelectionManager> customSelectionManager;

	void setLength(float value, bool stretch = false, bool stickToEnd = false);

	Array<float> getValuesForPosition(float pos);
	Array<float> getNormalizedValuesForPosition(float pos);

	AutomationKey * createItem() override;
	void addItems(Array<float> positions, Array<Array<float>> values, bool removeExistingOverlappingKeys = true, bool addToUndo = true, bool autoSmoothCurve = false);
	AutomationKey * addItem(const float position, const Array<float> value, bool addToUndo = true, bool reorder = false);
    Array<AutomationKey *> addItemsFromClipboard(bool showWarning = false) override;

	void removeKeysBetween(float start, float end);
	void removeAllSelectedKeys();

	void setSnapPositions(Array<float> positions);
	float getClosestSnapForPos(float pos, int start = -1, int end = -1);

	void clearRange();
	void setRange(Array<float> minValue, Array<float> maxValue);

	AutomationKey * getClosestKeyForPos(float pos, int start = -1, int end = -1);
	AutomationKey * getKeyAtPos(float pos);

	virtual void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable *c) override;
	virtual void onContainerParameterChanged(Parameter *) override;

	static int compareTime(AutomationKey * t1, AutomationKey * t2);

	InspectableEditor * getEditor(bool isRoot) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Automation)
};
