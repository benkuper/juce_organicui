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
	StringParameter(const String& niceName, const String& description, const String& initialValue, bool enabled = true);
	virtual ~StringParameter();

	enum UIType { TEXT, FILE };
	UIType defaultUI;

	bool multiline;
	String prefix;
	String suffix;
	bool autoTrim; //auto remove start and end whitespaces after edit

	virtual var getLerpValueTo(var targetValue, float weight) override;


	// need to override this function because var Strings comparison  is based on pointer (we need full string comp)
	virtual  void setValueInternal(var&)override;

	virtual bool checkValueIsTheSame(var oldValue, var newValue) override;

	virtual void setAttribute(String param, var paramVal) override;
	virtual StringArray getValidAttributes() const override;

	StringParameterUI* createStringParameterUI(Array<StringParameter*> parameters = {});
	StringParameterUI* createStringParameterFileUI(Array<StringParameter*> parameters = {});
	StringParameterTextUI* createStringParameterTextUI(Array<StringParameter*> parameters = {});
	ControllableUI* createDefaultUI(Array<Controllable*> controllables = {}) override;

	static StringParameter* create() { return new StringParameter("New StringParameter", "", ""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "String"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameter)
};