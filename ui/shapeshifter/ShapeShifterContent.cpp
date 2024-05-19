/*
  ==============================================================================

	ShapeShifterContent.cpp
	Created: 3 May 2016 10:17:41am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "ShapeShifterContent.h"

ShapeShifterContent::ShapeShifterContent(Component* _contentComponent, const String& _contentName) :
	contentComponent(_contentComponent),
	contentName(_contentName),
	contentIsFlexible(false),
	contentIsShown(false),
	sscNotifier(5)
{
	customName = "";
}

ShapeShifterContent::~ShapeShifterContent()
{
	DBG("here");
}

void ShapeShifterContent::setCustomName(const juce::String& newName)
{
	customName = newName;
	sscNotifier.addMessage(new ShapeShifterContentEvent(ShapeShifterContentEvent::NAME_CHANGED, this));
}

ShapeShifterContentComponent::ShapeShifterContentComponent(const String& contentName) :
	ShapeShifterContent(this, contentName)
{

}

void ShapeShifterContentComponent::paint(Graphics& g)
{
	if (ShapeShifterPanel* p = dynamic_cast<ShapeShifterPanel*>(getParentComponent()))
	{
		if (!p->isDetached())
		{
			g.fillAll(BG_COLOR);
		}
	}
}

void ShapeShifterContentComponent::mouseEnter(const MouseEvent& e)
{
	HelpBox::getInstance()->setOverData(helpID);
}

void ShapeShifterContentComponent::mouseExit(const MouseEvent& e)
{
	HelpBox::getInstance()->clearOverData(helpID);
}
