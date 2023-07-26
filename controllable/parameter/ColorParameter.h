/*
  ==============================================================================

    ColorParameter.h
    Created: 11 Apr 2017 9:33:55am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ColorParameterUI;

class ColorParameter :
	public Parameter
{
public:

	enum Mode { UINT, FLOAT };
	ColorParameter(const juce::String& niceName, const juce::String& description, const juce::Colour& initialColor = juce::Colours::black, bool enabled = true);
	~ColorParameter();

	Mode mode;

	const juce::Colour getColor();
	void setFloatRGBA(const float& r, const float& g, const float& b, const float& a);
	void setColor(const juce::uint32& _color, bool silentSet = false, bool force = false);
	void setColor(const juce::Colour& _color, bool silentSet = false, bool force = false);

	void setDefaultValue(const juce::Colour& _color, bool doResetValue = true);

	virtual juce::StringArray getValuesNames() override;

	bool checkValueIsTheSame(juce::var oldValue, juce::var newValue) override;

	virtual juce::var getLerpValueTo(juce::var targetValue, float weight) override;
	virtual void setWeightedValue(juce::Array<juce::var> values, juce::Array<float> weights) override;

	void setControlAutomation() override;

	juce::var getRemoteControlValue() override;

	ColorParameterUI* createColorParamUI(juce::Array<ColorParameter*> colorParameters = {});
	ControllableUI* createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	static ColorParameter * create() { return new ColorParameter("New Color Parameter", ""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Color"; }

};