/*
  ==============================================================================

	GradientColorUI.cpp
	Created: 11 Apr 2017 11:40:31am
	Author:  Ben

  ==============================================================================
*/


GradientColorUI::GradientColorUI(GradientColor* item) :
	ItemMinimalUI(item)
{
	dragAndDropEnabled = false;

	autoDrawContourWhenSelected = false;
	colorUI.reset(item->itemColor->createColorParamUI());
	addAndMakeVisible(colorUI.get());
	//colorUI->addMouseListener(this, false); //why ??
	colorUI->showMenuOnRightClick = false;
}

GradientColorUI::~GradientColorUI()
{

}

void GradientColorUI::paint(Graphics& g)
{
	g.setColour(item->isSelected ? HIGHLIGHT_COLOR : (item->isPreselected ? BLUE_COLOR : bgColor));
	g.fillPath(drawPath);
	g.setColour(item->isSelected ? HIGHLIGHT_COLOR.brighter() : (item->isPreselected ? BLUE_COLOR.brighter() : bgColor.darker(.7f)));
	g.strokePath(drawPath, PathStrokeType(1));
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

	colorUI->setBounds(getLocalBounds().reduced(2, getHeight() / 4));
}

void GradientColorUI::mouseDown(const MouseEvent& e)
{

	ItemMinimalUI::mouseDown(e);



	if (e.mods.isCommandDown())
	{
		item->interpolation->setNext(true, true);
	}
	else if (e.mods.isRightButtonDown())
	{
		//Need to fix multi level

		GradientColor::Interpolation i = item->interpolation->getValueDataAsEnum<GradientColor::Interpolation>();
		PopupMenu p;
		PopupMenu ip;
		ip.addItem(1, "Linear", true, i == GradientColor::LINEAR);
		ip.addItem(2, "None", true, i == GradientColor::NONE);
		p.addSubMenu("Interpolation", ip);

		p.showMenuAsync(PopupMenu::Options(), [this](int result)
			{
				switch (result)
				{
				case 1: this->item->interpolation->setValueWithData(GradientColor::LINEAR); break;
				case 2: this->item->interpolation->setValueWithData(GradientColor::NONE); break;
				}
			}
		);
	}
	else if (e.eventComponent != this)
	{
		if (item->isUILocked->boolValue()) return;
		item->setMovePositionReference(true);
	}
}