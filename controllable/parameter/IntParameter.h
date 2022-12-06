/*
  ==============================================================================

    IntParameter.h
    Created: 8 Mar 2016 1:22:23pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class IntSliderUI;
class IntStepperUI;
class IntParameterLabelUI;

class IntParameter : public Parameter
{
public:
    IntParameter(const String &niceName, const String &description, const int &initialValue, const int &minimumValue = INT32_MIN, const int &maximumValue = INT32_MAX, bool enabled = true);
    ~IntParameter() {}

	bool hexMode;

	virtual void setValueInternal(var & _value) override;

	virtual var getLerpValueTo(var targetValue, float weight) override;
	virtual void setWeightedValue(Array<var> values, Array<float> weights) override;
	var getCroppedValue(var originalValue) override;

	virtual bool hasRange() override;

	void setControlAutomation() override;

	bool setAttributeInternal(String attribute, var val) override;
	virtual StringArray getValidAttributes() const override;

	var getJSONDataInternal() override;
	void loadJSONDataInternal(var data) override;

	IntSliderUI* createSlider(Array<Parameter*> parameters = {});
	IntStepperUI * createStepper(Array<Parameter*> parameters = {});
	IntParameterLabelUI * createLabelUI(Array<Parameter*> parameters = {});
    ControllableUI * createDefaultUI(Array<Controllable*> controllables = {}) override;

	static IntParameter * create() { return new IntParameter("New Int Parameter", "", 0); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Integer"; }
};