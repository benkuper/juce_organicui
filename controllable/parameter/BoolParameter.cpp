/*
  ==============================================================================

    BoolParameter.cpp
    Created: 8 Mar 2016 1:22:15pm
    Author:  bkupe

  ==============================================================================
*/

BoolParameter::BoolParameter(const String & niceName, const String &description, bool initialValue, bool enabled) :
    Parameter(Type::BOOL, niceName, description, initialValue, 0, 1, enabled)
{
	minimumValue = 0;
	maximumValue = 1;
	canHaveRange = true;
    setValue(initialValue);
	argumentsDescription = "0/1";
}

void BoolParameter::setValueInternal(var& _value)
{
	value = (bool)(int)_value;
}

BoolToggleUI * BoolParameter::createToggle(Image onImage, Image offImage)
{
    return new BoolToggleUI(this, onImage, offImage);
}

BoolButtonToggleUI* BoolParameter::createButtonToggle()
{
	return new BoolButtonToggleUI(this);
}

ControllableUI * BoolParameter::createDefaultUI() {
	return createToggle();
}
