#include "BoolToggleUI.h"
/*
  ==============================================================================

    BoolToggleUI.cpp
    Created: 8 Mar 2016 3:47:01pm
    Author:  bkupe

  ==============================================================================
*/

BoolToggleUI::BoolToggleUI(Parameter * parameter) :
    ParameterUI(parameter), invertVisuals(false)
{
	showEditWindowOnDoubleClick = false;

	if (!isInteractable() && (parameter->enabled || parameter->isControllableFeedbackOnly)) 
	{
		setImages(ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_on_readonly_png, OrganicUIBinaryData::checkbox_on_readonly_pngSize), ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_off_readonly_png, OrganicUIBinaryData::checkbox_off_readonly_pngSize));
	}else
	{
		setImages(ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_on_png, OrganicUIBinaryData::checkbox_on_pngSize), ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_off_png, OrganicUIBinaryData::checkbox_off_pngSize));
		setRepaintsOnMouseActivity(true);
	}

    setSize(10,10);
    
#if JUCE_MAC
    startTimerHz(20); //20 fps for slider on mac because of bad UI handling
#else
    startTimerHz(30); //30 fps for slider
#endif
    
}

BoolToggleUI::~BoolToggleUI()
{

}

void BoolToggleUI::setImages(Image _onImage, Image _offImage)
{
	onImage = _onImage;
	offImage = _offImage;
}

void BoolToggleUI::paint(Graphics & g)
{
    // we are on component deletion
    if(shouldBailOut())return;

	bool valCheck = invertVisuals ? !parameter->boolValue():parameter->boolValue();
	Image m = valCheck ? onImage : offImage;
	
 juce::Rectangle<int> r = getLocalBounds();
	g.setColour(Colours::white.withAlpha(isMouseOver() ? 1 : .8f));

 juce::Rectangle<int> cr;
	if (showLabel)
	{
		cr = r.removeFromRight(r.getHeight());
		r.removeFromRight(2);
	}else
	{
		cr = r.removeFromLeft(r.getHeight());
	}

	g.drawImage(m, cr.toFloat());
	drawRect = cr;
	
	/*
    g.setGradientFill(ColourGradient(c.brighter(.2f),(float)getLocalBounds().getCentreX(),(float)getLocalBounds().getCentreY(), c.darker(.2f), 2.f,2.f,true));
    g.fillRoundedRectangle(getLocalBounds().toFloat(),2);
	*/

	if (showLabel)
	{
		g.setFont((float)jmin<int>(getHeight(),12));
		g.setColour(Colours::white);
		g.drawFittedText(parameter->niceName, r, Justification::left,1);
	}
}

void BoolToggleUI::mouseDownInternal(const MouseEvent & e)
{
	if (!isInteractable()) return;
	if (e.mods.isRightButtonDown()) parameter->setValue(parameter->boolValue(), !parameter->boolValue());
	else parameter->setUndoableValue(parameter->boolValue(), !parameter->boolValue()); //only undoable when from left button, real toggle behaviour
}

void BoolToggleUI::mouseUpInternal(const MouseEvent & e)
{
	if (isInteractable()) return;
    if (e.mods.isRightButtonDown()) parameter->setValue(!parameter->boolValue());
}

bool BoolToggleUI::hitTest(int x, int y)
{
	return drawRect.contains(x, y);
}



void BoolToggleUI::valueChanged(const var & )
{
    shouldRepaint = true;
}

void BoolToggleUI::timerCallback()
{
    if (!shouldRepaint) return;
    shouldRepaint = false;
    repaint();
    
}

