#include "StringParameter.h"
/*
  ==============================================================================

    StringParameter.cpp
    Created: 9 Mar 2016 12:29:30am
    Author:  bkupe

  ==============================================================================
*/


StringParameter::StringParameter(const String & niceName, const String &description, const String & initialValue, bool enabled) :
    Parameter(Type::STRING, niceName, description, initialValue, var(), var(), enabled),
	defaultUI(TEXT),
	multiline(false)
{
	argumentsDescription = "string";
	isCustomizableByUser = false; //avoid having the param wheel 
}

StringParameter::~StringParameter()
{
}

StringParameterUI * StringParameter::createStringParameterUI(StringParameter * target)
{
	if (target == nullptr) target = this;
    return new StringParameterUI(target);
}


StringParameterUI * StringParameter::createStringParameterFileUI(StringParameter * target)
{
	if (target == nullptr) target = this;
	return new StringParameterFileUI(target);
}

StringParameterTextUI * StringParameter::createStringParameterTextUI(StringParameter * target)
{
	if (target == nullptr) target = this;
	return new StringParameterTextUI(target);
}

ControllableUI* StringParameter::createDefaultUI(Controllable * targetControllable){

	switch (defaultUI)
	{
	case TEXT: 
	{
		if (!multiline) return createStringParameterUI(dynamic_cast<StringParameter *>(targetControllable)); 
		else return createStringParameterTextUI(dynamic_cast<StringParameter *>(targetControllable));
	}
	break;

	case  FILE: return createStringParameterFileUI(dynamic_cast<StringParameter *>(targetControllable)); break;
	}

	return createStringParameterUI(dynamic_cast<StringParameter *>(targetControllable));
}

var StringParameter::getLerpValueTo(var targetValue, float weight)
{
	//TODO implement levestein mes couilles plus courte distance entre 2 mots 
	return Parameter::getLerpValueTo(targetValue, weight);
}

void  StringParameter::setValueInternal(var & newVal)
{
	
	if (newVal.isArray())
	{
		String v = "";
		for (int i = 0; i < newVal.size(); i++) v += (i > 0 ? " " : "") + newVal[i].toString();
		value = v;
	} else
	{
		value = newVal.toString();
	}    
}
bool StringParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	return oldValue.toString() == newValue.toString();
}
;

