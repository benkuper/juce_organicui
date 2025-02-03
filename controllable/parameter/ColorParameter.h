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

	ColorParameter(const juce::String& niceName, const juce::String& description, const juce::Colour& initialColor = juce::Colours::black, bool enabled = true);
	~ColorParameter();

	juce::Colour getColor() const;
	void setFloatRGBA(const float& r, const float& g, const float& b, const float& a, bool setSimilarSelected = false);
	void setColor(const juce::uint32& _color, bool silentSet = false, bool force = false, bool setSimilarSelected = false);
	void setColor(const juce::Colour& _color, bool silentSet = false, bool force = false, bool setSimilarSelected = false);

	void setBounds(float _minR, float _minG, float _minB, float _minA, float _maxR, float _maxG, float _maxB, float _maxA);
	void clearRange() override;
	bool hasRange() const override;

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

protected:
	juce::var getCroppedValue(juce::var originalValue) override;
};
