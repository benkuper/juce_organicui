/*
  ==============================================================================

    Automation.h
    Created: 11 Dec 2016 1:21:37pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class InspectableSelectionManager;

class AutomationKeyComparator
{
public:
	int compareElements(AutomationKey * t1, AutomationKey * t2)
	{
		if (t1->position->floatValue() < t2->position->floatValue()) return -1;
		else if (t1->position->floatValue() > t2->position->floatValue()) return 1;
		return 0;
	}
};

class Automation :
	public BaseManager<AutomationKey>
{
public:
	Automation(const String &name = "Automation", AutomationRecorder * recorder = nullptr, bool freeRange = false, bool allowKeysOutside = false, bool dedicatedSelectionManager = true);
	virtual ~Automation();


	//Recorder
	AutomationRecorder * recorder;

	//ui
	bool showUIInEditor;

	//Position and value
	bool freeRange;
	bool allowKeysOutside; //allow keys positions to be outside automation timing
	FloatParameter * position;
	FloatParameter * value;
	FloatParameter * length;

	//snapping
	Array<float> snapPositions;
	BoolParameter * enableSnap;
	FloatParameter * snapSensitivity;

	ScopedPointer<InspectableSelectionManager> customSelectionManager;

	float getValueForPosition(float pos);
	float getNormalizedValueForPosition(float pos);

	static AutomationKeyComparator comparator;
	
	AutomationKey * createItem() override;
	void addItems(Array<Point<float>> keys, bool removeExistingOverlappingKeys = true, bool addToUndo = true, bool autoSmoothCurve = false);
	void addItem(const float position, const float value, bool addToUndo = true, bool reorder = false);
	void reorderItems() override;

	void removeKeysBetween(float start, float end);
	void removeAllSelectedKeys();

	void setSnapPositions(Array<float> positions);

	float getClosestSnapForPos(float pos, int start = -1, int end = -1);

	void clearRange();
	void setRange(float minValue, float maxValue);

	AutomationKey * getClosestKeyForPos(float pos, int start = -1, int end = -1);
	AutomationKey * getKeyAtPos(float pos);

	virtual void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable *c) override;
	virtual void onContainerParameterChanged(Parameter *) override;

	InspectableEditor * getEditor(bool isRoot) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Automation)
};
