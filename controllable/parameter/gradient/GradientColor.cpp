/*
  ==============================================================================

    TimeColor.cpp
    Created: 11 Apr 2017 11:40:21am
    Author:  Ben

  ==============================================================================
*/

GradientColor::GradientColor(float _time, const Colour & _color, const String & name) :
	BaseItem(name,false),
	gradientIndex(-1)
{
	itemDataType = "GradientColor";
	
	position = addFloatParameter("Time", "Time for the color",0);
	color = new ColorParameter("Color", "Color of the item", Colours::black);
	interpolation = addEnumParameter("Interpolation", "Interpolation to the next key");
	interpolation->addOption("Linear", Interpolation::LINEAR)->addOption("None", Interpolation::NONE);

	position->setValue(_time);
	color->setColor(_color);
	addParameter(color);
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
	actions.add(position->setUndoableValue(movePositionReference.x, position->floatValue(), true));
}
