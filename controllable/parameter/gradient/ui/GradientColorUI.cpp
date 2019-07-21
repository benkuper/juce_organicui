/*
  ==============================================================================

    GradientColorUI.cpp
    Created: 11 Apr 2017 11:40:31am
    Author:  Ben

  ==============================================================================
*/


GradientColorUI::GradientColorUI(GradientColor * item) :
	BaseItemMinimalUI(item)
{
	dragAndDropEnabled = false;

	autoDrawContourWhenSelected = false;
	colorUI.reset(item->color->createColorParamUI());
	addAndMakeVisible(colorUI.get());
	//colorUI->addMouseListener(this, false); //why ??
}

GradientColorUI::~GradientColorUI()
{
	
}

void GradientColorUI::paint(Graphics & g)
{
	g.setColour(item->isSelected ? HIGHLIGHT_COLOR:bgColor);
	g.fillPath(drawPath);
	g.setColour(item->isSelected?HIGHLIGHT_COLOR.brighter():bgColor.darker(.7f));
	g.strokePath(drawPath,PathStrokeType(1));
}

void GradientColorUI::resized()
{
	drawPath.clear();
	drawPath.startNewSubPath((float)getWidth() / 2, 0);
	drawPath.lineTo((float)getWidth(), 4);
	drawPath.lineTo((float)getWidth(), (float)getHeight());
	drawPath.lineTo(0, (float)getHeight());
	drawPath.lineTo(0, 4);
	drawPath.closeSubPath();
	drawPath = drawPath.createPathWithRoundedCorners(1);

	colorUI->setBounds(getLocalBounds().reduced(2, getHeight()/4));
}

void GradientColorUI::mouseDown(const MouseEvent & e)
{
	BaseItemMinimalUI::mouseDown(e);
	posAtMouseDown = item->position->floatValue();
	if (e.mods.isCommandDown())
	{
		item->interpolation->setNext(true, true);
	}
}

void GradientColorUI::mouseUp(const MouseEvent & e)
{
	item->position->setUndoableValue(posAtMouseDown, item->position->floatValue());
}
