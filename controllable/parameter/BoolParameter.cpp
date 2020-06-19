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

BoolToggleUI * BoolParameter::createToggle(BoolParameter * target)
{
	if (target == nullptr) target = this;
    return new BoolToggleUI(target);
}

BoolImageToggleUI * BoolParameter::createImageToggle(ImageButton * bt, BoolParameter * target)
{
	if (target == nullptr) target = this;
	return new BoolImageToggleUI(bt, target);
}


ControllableUI * BoolParameter::createDefaultUI() {
	return createToggle(this);
}
