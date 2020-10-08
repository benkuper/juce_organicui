#include "BoolParameter.h"
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

BoolToggleUI * BoolParameter::createToggle()
{
    return new BoolToggleUI(this);
}

BoolButtonToggleUI* BoolParameter::createButtonToggle()
{
	return new BoolButtonToggleUI(this);
}

BoolImageToggleUI * BoolParameter::createImageToggle(ImageButton * bt)
{
	return new BoolImageToggleUI(bt, this);
}

ControllableUI * BoolParameter::createDefaultUI() {
	return createToggle();
}
