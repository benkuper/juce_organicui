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
	multiline(false),
	autoTrim(false)
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

ControllableUI* StringParameter::createDefaultUI(){

	switch (defaultUI)
	{
	case TEXT: 
	{
		if (!multiline) return createStringParameterUI(this); 
		else return createStringParameterTextUI(this);
	}
	break;

	case  FILE: return createStringParameterFileUI(this); break;
	}

	return createStringParameterUI(this);
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
		for (int i = 0; i < newVal.size(); i++) v += (i > 0 ? " " : "") + String((float)newVal[i],3,0);
		value = v;
	} else
	{
		value = newVal.isString()?newVal.toString():String((float)newVal,3);
	}    
}
bool StringParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	return oldValue.toString() == newValue.toString();
}

void StringParameter::setAttribute(String param, var paramVal)
{
	Parameter::setAttribute(param, paramVal);

	if (param == "multiline") multiline = paramVal;
	else if (param == "prefix") prefix = paramVal;
	else if (param == "suffix") suffix = paramVal;
}

