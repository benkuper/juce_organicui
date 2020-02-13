/*
  ==============================================================================

    AutomationKey.h
    Created: 11 Dec 2016 1:22:20pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationKey :
	public BaseItem,
	public Inspectable::InspectableListener
{
public:
	AutomationKey(int numDimensions = 1, Array<float> minimumValues = Array<float>((float)INT32_MIN), Array<float> maximumValue = Array<float>((float)INT32_MAX));
	virtual ~AutomationKey();

	int numDimensions;

	FloatParameter * position; //depends on parent automation
	Array<FloatParameter *> values; //0-1

	EnumParameter * easingType;

	OwnedArray<Easing> easings;

	AutomationKey * nextKey;


	void clearItem() override;

	void setNextKey(AutomationKey* k);
	
	void clearRange();
	void setRange(Array<float> minVal, Array<float> maxVal);

	void setEasing(Easing::Type t);

	float getDimensionValue(int index);
	Array<float> getValues(const float &position = 0);
	void setValues(Array<float> newValues, bool updatePosition = true);

	void setSelectionManager(InspectableSelectionManager * ism) override;
	void onContainerParameterChangedInternal(Parameter *) override;
	void controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

	void inspectableDestroyed(Inspectable* i) override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationKey)
};

