/*
  ==============================================================================

    Easing.cpp
    Created: 11 Dec 2016 1:29:02pm
    Author:  Ben

  ==============================================================================
*/

EasingBase::EasingBase(Type _type) :
ControllableContainer("ease"),
type(_type)
{
	//showInspectorOnSelect = false;

	helpID = "Easing";
}

EasingBase::~EasingBase()
{
	masterReference.clear();
}

