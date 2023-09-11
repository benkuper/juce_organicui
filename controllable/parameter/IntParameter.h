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
    IntParameter(const juce::String &niceName, const juce::String &description, const int &initialValue, const int &minimumValue = INT32_MIN, const int &maximumValue = INT32_MAX, bool enabled = true);
    ~IntParameter() {}

	bool hexMode;

	virtual void setValueInternal(juce::var& _value) override;

	virtual juce::var getLerpValueTo(juce::var targetValue, float weight) override;
	virtual void setWeightedValue(juce::Array<juce::var> values, juce::Array<float> weights) override;
	juce::var getCroppedValue(juce::var originalValue) override;

	virtual bool hasRange() override;

	void setControlAutomation() override;

	bool setAttributeInternal(juce::String attribute, juce::var val) override;
	virtual juce::StringArray getValidAttributes() const override;

	juce::var getJSONDataInternal() override;
	void loadJSONDataInternal(juce::var data) override;

	IntSliderUI* createSlider(juce::Array<Parameter*> parameters = {});
	IntStepperUI* createStepper(juce::Array<Parameter*> parameters = {});
	IntParameterLabelUI* createLabelUI(juce::Array<Parameter*> parameters = {});
	ControllableUI* createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	static IntParameter * create() { return new IntParameter("New Int Parameter", "", 0); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Integer"; }
};