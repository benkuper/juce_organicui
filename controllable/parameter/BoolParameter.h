/*
  ==============================================================================

    BoolParameter.h
    Created: 8 Mar 2016 1:22:15pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BoolToggleUI;
class BoolButtonToggleUI;

class BoolParameter : public Parameter
{
public:
    BoolParameter(const juce::String &niceName, const juce::String &description, bool initialValue, bool enabled = true);
    ~BoolParameter() {}

    //ui creation

    void setValueInternal(juce::var& value) override;

    BoolToggleUI * createToggle(juce::Image onImage = juce::Image(), juce::Image offImage = juce::Image(), juce::Array<BoolParameter*> c = {});
	BoolButtonToggleUI* createButtonToggle(juce::Array<BoolParameter *> c = {});
    ControllableUI* createDefaultUI(juce::Array<Controllable*> c = {}) override;

	static BoolParameter * create() { return new BoolParameter("New Bool Parameter", "", false); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Boolean"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoolParameter)
};