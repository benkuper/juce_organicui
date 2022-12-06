/*
  ==============================================================================

	StringParameter.cpp
	Created: 9 Mar 2016 12:29:30am
	Author:  bkupe

  ==============================================================================
*/


StringParameter::StringParameter(const String& niceName, const String& description, const String& initialValue, bool enabled) :
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

StringParameterUI* StringParameter::createStringParameterUI(Array<StringParameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new StringParameterUI(parameters);
}


StringParameterUI* StringParameter::createStringParameterFileUI(Array<StringParameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new StringParameterFileUI(parameters);
}

StringParameterTextUI* StringParameter::createStringParameterTextUI(Array<StringParameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new StringParameterTextUI(parameters);
}

ControllableUI* StringParameter::createDefaultUI(Array<Controllable*> controllables) {

	Array<StringParameter*> parameters = getArrayAs<Controllable, StringParameter>(controllables);

	switch (defaultUI)
	{
	case TEXT:
	{
		if (!multiline) return createStringParameterUI(parameters);
		else return createStringParameterTextUI(parameters);
	}
	break;

	case  FILE: return createStringParameterFileUI(parameters); break;
	}

	return createStringParameterUI(parameters);
}

var StringParameter::getLerpValueTo(var targetValue, float weight)
{
	//TODO implement levestein mes couilles plus courte distance entre 2 mots 
	return Parameter::getLerpValueTo(targetValue, weight);
}

void  StringParameter::setValueInternal(var& newVal)
{

	if (newVal.isArray())
	{
		String v = "";
		for (int i = 0; i < newVal.size(); ++i) v += (i > 0 ? " " : "") + String((float)newVal[i], 3, 0);
		value = v;
	}
	else
	{
		value = newVal.isString() ? newVal.toString() : (newVal.isInt() || newVal.isInt64() ? String((int)newVal) : String((float)newVal, 3));
	}
}
bool StringParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	return oldValue.toString() == newValue.toString();
}

bool StringParameter::setAttributeInternal(String param, var paramVal)
{

	if (param == "multiline") multiline = paramVal;
	else if (param == "prefix") prefix = paramVal;
	else if (param == "suffix") suffix = paramVal;
	else
	{
		return Parameter::setAttributeInternal(param, paramVal);
	}

	return true;
}

StringArray StringParameter::getValidAttributes() const
{
	StringArray att = Parameter::getValidAttributes();
	att.addArray({ "multiline","prefix","suffix" });
	return att;
}

