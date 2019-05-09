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

class FloatParameter : public Parameter
{
public:
    FloatParameter(const String &niceName, const String &description, const float &initialValue, const float &minValue = (float)INT32_MIN, const float &maxValue = (float)INT32_MAX, bool enabled = true);
    ~FloatParameter() {}

    void setValueInternal(var & _value) override;

	virtual var getLerpValueTo(var targetValue, float weight) override;
	virtual void setWeightedValue(Array<var> values, Array<float> weights) override;

	enum UIType {NONE, SLIDER, STEPPER, LABEL, TIME };
	UIType defaultUI;
	UIType customUI;

    FloatSliderUI * createSlider(FloatParameter * target = nullptr);
    FloatStepperUI * createStepper(FloatParameter * target = nullptr);
	FloatParameterLabelUI * createLabelParameter(FloatParameter * target = nullptr);
	TimeLabel * createTimeLabelParameter(FloatParameter * target = nullptr);

    ControllableUI * createDefaultUI(Controllable * targetControllable = nullptr) override;

	bool checkValueIsTheSame(var oldValue, var newValue) override;

	virtual bool hasRange() override;

	virtual String stringValue() override { return String(floatValue()); }

	var getJSONDataInternal() override;
	void loadJSONDataInternal(var data) override;

	static FloatParameter * create() { return new FloatParameter("New Float Parameter", "",0); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Float"; }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatParameter)
};
