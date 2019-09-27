#include "ColorParameterUI.h"
/*
  ==============================================================================

	ColorParameterUI.cpp
	Created: 11 Apr 2017 10:42:03am
	Author:  Ben

  ==============================================================================
*/


ColorParameterUI::ColorParameterUI(ColorParameter * parameter) :
	ParameterUI(parameter),
    colorParam(parameter),
    dispatchOnDoubleClick(true),
	dispatchOnSingleClick(false)
{

}

ColorParameterUI::~ColorParameterUI()
{
}

void ColorParameterUI::paint(Graphics & g)
{
	Colour c = colorParam->getColor();
	int size = jmin(getWidth(), getHeight()) / 2;
	if (!c.isOpaque()) g.fillCheckerBoard(getLocalBounds().reduced(1).toFloat(), size, size, Colours::white, Colours::white.darker(.2f));
	
	g.setColour(c);
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 2);

	if (isInteractable())
	{
		g.setColour(c.brighter(.5f));
		g.drawRoundedRectangle(getLocalBounds().reduced(1).toFloat(), 2, 2);
	}

	if (showLabel)
	{
		Rectangle<int> tr = getLocalBounds().reduced(2);
		g.setFont(jlimit(12, 40, jmin(tr.getHeight(), tr.getWidth()) - 16));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : colorParam->niceName, tr, Justification::centred, 1);
	}
	
}

void ColorParameterUI::resized()
{

}

void ColorParameterUI::mouseDownInternal(const MouseEvent & e)
{
	if (dispatchOnSingleClick) showEditWindow();
}

void ColorParameterUI::showEditWindowInternal()
{
	if (!isInteractable()) return;

	ColourSelector * selector = new ColourSelector();
	selector->addChangeListener(this);
	selector->setName("Color for " + parameter->niceName);
	selector->setCurrentColour(colorParam->getColor());
	selector->setColour(ColourSelector::backgroundColourId, Colours::transparentBlack);
	selector->setSize(300, 400);
	selector->addChangeListener(this);
	CallOutBox::launchAsynchronously(selector, getScreenBounds(), nullptr);
}

void ColorParameterUI::valueChanged(const var &)
{
	repaint();
}

void ColorParameterUI::changeListenerCallback(ChangeBroadcaster * source)
{
	ColourSelector * s = dynamic_cast<ColourSelector *>(source);
	if (s == nullptr) return;
	colorParam->setColor(s->getCurrentColour());

}

