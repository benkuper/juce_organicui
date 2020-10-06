/*
  ==============================================================================

    BoolParameter.h
    Created: 8 Mar 2016 1:22:15pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BoolToggleUI;
class BoolImageToggleUI;
class BoolButtonToggleUI;

class BoolParameter : public Parameter
{
public:
    BoolParameter(const String &niceName, const String &description, bool initialValue, bool enabled = true);
    ~BoolParameter() {}

    //ui creation
    BoolToggleUI * createToggle();
	BoolButtonToggleUI* createButtonToggle();
	BoolImageToggleUI * createImageToggle(ImageButton * image);
	ControllableUI * createDefaultUI() override;

	static BoolParameter * create() { return new BoolParameter("New Bool Parameter", "", false); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Boolean"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoolParameter)
};