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
	ColorParameter(const String &niceName, const String &description, const Colour &initialColor = Colours::black, bool enabled = true);
	~ColorParameter();

	Mode mode;

	const Colour getColor();
	void setFloatRGBA(const float &r, const float &g, const float &b, const float &a);
	void setColor(const uint32 &_color, bool silentSet = false, bool force = false);
	void setColor(const Colour &_color, bool silentSet = false, bool force = false);

	virtual StringArray getValuesNames() override;

	bool checkValueIsTheSame(var oldValue, var newValue) override;

	virtual var getLerpValueTo(var targetValue, float weight) override;
	virtual void setWeightedValue(Array<var> values, Array<float> weights) override;

	void setControlAutomation() override;

	ColorParameterUI * createColorParamUI();
	ControllableUI * createDefaultUI(Controllable * targetControllable = nullptr) override;

	static ColorParameter * create() { return new ColorParameter("New Color Parameter", ""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Color"; }

};