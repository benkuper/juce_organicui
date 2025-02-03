/*
  ==============================================================================

	TimeColor.cpp
	Created: 11 Apr 2017 11:40:21am
	Author:  Ben

  ==============================================================================
*/

GradientColor::GradientColor(float _time, const Colour& _color, const String& name) :
	BaseItem(name, false),
	gradientIndex(-1)
{
	itemDataType = "GradientColor";

	position = addFloatParameter("Time", "Time for the color", 0);

	setHasCustomColor(true);
	itemColor->setDefaultValue(Colours::black);
	
	interpolation = addEnumParameter("Interpolation", "Interpolation to the next key");
	interpolation->addOption("Linear", Interpolation::LINEAR)->addOption("None", Interpolation::NONE);

	position->setValue(_time);

	itemColor->setDefaultValue(Colours::black, false);
	itemColor->setColor(_color);
}

GradientColor::~GradientColor()
{

}

void GradientColor::setMovePositionReferenceInternal()
{
	movePositionReference.setX(position->floatValue());
}

void GradientColor::setPosition(Point<float> targetPosition)
{
	position->setValue(targetPosition.x);
}

Point<float> GradientColor::getPosition()
{
	return Point<float>(position->floatValue(), 0);
}

void GradientColor::addUndoableMoveAction(Array<UndoableAction*>& actions)
{
	actions.addArray(position->setUndoableValue(position->floatValue(), true));
}
