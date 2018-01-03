/*
  ==============================================================================

    StringParameter.cpp
    Created: 9 Mar 2016 12:29:30am
    Author:  bkupe

  ==============================================================================
*/


StringParameter::StringParameter(const String & niceName, const String &description, const String & initialValue, bool enabled) :
    Parameter(Type::STRING, niceName, description, initialValue, var(), var(), enabled),
	defaultUI(TEXT)
{
	argumentsDescription = "string";
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

ControllableUI* StringParameter::createDefaultUI(Controllable * targetControllable){

	switch (defaultUI)
	{
	case TEXT: return createStringParameterUI(dynamic_cast<StringParameter *>(targetControllable)); break;
	case  FILE: return createStringParameterFileUI(dynamic_cast<StringParameter *>(targetControllable)); break;
	}

	return createStringParameterUI(dynamic_cast<StringParameter *>(targetControllable));
};


void StringParameter::setValue(var _value,bool silentSet,bool force )
{
    if (!force && value.toString() == _value.toString()) return;
    
    setValueInternal(_value);
    
    if(_value != defaultValue) isOverriden = true;
    
    if (!silentSet) notifyValueChanged();
};

void  StringParameter::setValueInternal(var & newVal){
	
    value = newVal.toString();
};

