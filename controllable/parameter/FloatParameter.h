/*
 ==============================================================================

 FloatParameter.h
 Created: 8 Mar 2016 1:22:10pm
 Author:  bkupe

 ==============================================================================
 */

#pragma once

class FloatSliderUI;
class FloatStepperUI;
class FloatParameterLabelUI;
class TimeLabel;

#define DEFAULT_STRING_DECIMALS 3

class FloatParameter : public Parameter
{
public:
	FloatParameter(const juce::String& niceName, const juce::String& description, const double& initialValue, const double& minValue = (double)INT32_MIN, const double& maxValue = (double)INT32_MAX, bool enabled = true);
	virtual ~FloatParameter() {}

	double unitSteps;
	int stringDecimals;

	enum UIType { NONE, SLIDER, STEPPER, LABEL, TIME };
	UIType defaultUI;
	UIType customUI;

	FloatSliderUI* createSlider(juce::Array<Parameter*> parameters = {});
	FloatStepperUI* createStepper(juce::Array<Parameter*> parameters = {});
	FloatParameterLabelUI* createLabelParameter(juce::Array<Parameter*> parameters = {});
	TimeLabel* createTimeLabelParameter(juce::Array<Parameter*> parameters = {});

	virtual ControllableUI* createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	bool checkValueIsTheSame(juce::var oldValue, juce::var newValue) override;

	virtual void setValueInternal(juce::var& value) override;

	virtual bool hasRange() override;

	virtual juce::var getLerpValueTo(juce::var targetValue, float weight) override;
	virtual void setWeightedValue(juce::Array<juce::var> values, juce::Array<float> weights) override;
	virtual double getStepSnappedValueFor(double originalValue);

	void setControlAutomation() override;

	virtual juce::String stringValue() override;

	bool setAttributeInternal(juce::String name, juce::var val) override;
	virtual juce::StringArray getValidAttributes() const override;

	juce::var getJSONDataInternal() override;
	void loadJSONDataInternal(juce::var data) override;

	static FloatParameter* create() { return new FloatParameter("New Float Parameter", "", 0); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Float"; }

	virtual juce::var getCroppedValue(juce::var originalValue) override;
};
