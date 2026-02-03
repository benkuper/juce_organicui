/*
  ==============================================================================

	StringParameter.h
	Created: 9 Mar 2016 12:29:30am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class StringParameterUI;
class StringParameterTextUI;

class StringParameter : public Parameter
{
public:
	StringParameter(const juce::String& niceName, const juce::String& description, const juce::String& initialValue, bool enabled = true);
	virtual ~StringParameter();

	enum UIType { TEXT, FILE };
	UIType defaultUI;

	bool multiline;
	juce::String prefix;
	juce::String suffix;
	bool autoTrim; //auto remove start and end whitespaces after edit

	virtual juce::var getLerpValueTo(juce::var targetValue, float weight) override;


	// need to override this function because var Strings comparison  is based on pointer (we need full string comp)
	virtual  void setValueInternal(juce::var&)override;

	virtual bool checkValueIsTheSame(juce::var oldValue, juce::var newValue) override;

	virtual bool setAttributeInternal(juce::String param, juce::var paramVal) override;
	virtual juce::var getAttributeInternal(juce::String name) const override;
	virtual juce::StringArray getValidAttributes() const override;

	StringParameterUI* createStringParameterUI(juce::Array<StringParameter*> parameters = {});
	StringParameterUI* createStringParameterFileUI(juce::Array<StringParameter*> parameters = {});
	StringParameterTextUI* createStringParameterTextUI(juce::Array<StringParameter*> parameters = {});
	ControllableUI* createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	static StringParameter* create() { return new StringParameter("New StringParameter", "", ""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "String"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameter)
};