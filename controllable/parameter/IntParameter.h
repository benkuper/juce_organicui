/*
  ==============================================================================

    IntParameter.h
    Created: 8 Mar 2016 1:22:23pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef INTPARAMETER_H_INCLUDED
#define INTPARAMETER_H_INCLUDED

class IntSliderUI;
class IntStepperUI;
class IntParameterLabelUI;

class IntParameter : public Parameter
{
public:
    IntParameter(const String &niceName, const String &description, const int &initialValue, const int &minimumValue = INT32_MIN, const int &maximumValue = INT32_MAX, bool enabled = true);
    ~IntParameter() {}

    void setValueInternal(var & _value) override;
	virtual var getLerpValueTo(var targetValue, float weight) override;
	virtual void setWeightedValue(Array<var> values, Array<float> weights) override;

    IntSliderUI * createSlider(IntParameter * target = nullptr);
	IntStepperUI * createStepper(IntParameter * target = nullptr);
	IntParameterLabelUI * createLabelUI(IntParameter * target = nullptr);
    ControllableUI * createDefaultUI(Controllable * targetControllable = nullptr) override;

	static IntParameter * create() { return new IntParameter("New Int Parameter", "", 0); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Integer"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntParameter)
};


#endif  // INTPARAMETER_H_INCLUDED