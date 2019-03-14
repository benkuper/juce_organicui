#include "InspectableContentComponent.h"
/*
  ==============================================================================

	InspectableContentComponent.cpp
	Created: 9 May 2016 6:51:16pm
	Author:  bkupe

  ==============================================================================
*/

InspectableContentComponent::InspectableContentComponent(Inspectable * inspectable) :
	InspectableContent(inspectable),
	repaintOnSelectionChanged(true),
	rounderCornerSize(4),
	autoDrawContourWhenSelected(true),
	selectionContourColor(HIGHLIGHT_COLOR),
	autoSelectWithChildRespect(true),
    highlightColor(Colours::purple),
    bringToFrontOnSelect(true)
{
	setWantsKeyboardFocus(true);
	setMouseClickGrabsKeyboardFocus(true);
	inspectable->addAsyncInspectableListener(this);
}

InspectableContentComponent::~InspectableContentComponent()
{
	if (!inspectable.wasObjectDeleted())
	{
		inspectable->removeAsyncInspectableListener(this);

		inspectable->setSelected(false);

		for (auto & i : inspectable->linkedInspectables)
		{
			if (!i.wasObjectDeleted()) i->setHighlighted(false);
		}
	}
}


void InspectableContentComponent::mouseEnter(const MouseEvent & e)
{
	//DBG("Mouse Enter : " << inspectable->getHelpID());
	if (inspectable.wasObjectDeleted()) return;
	if (HelpBox::getInstance()->overDataID.isEmpty()) HelpBox::getInstance()->setOverData(inspectable->getHelpID());

	for (auto & i : inspectable->linkedInspectables)
	{
		if (!i.wasObjectDeleted()) i->setHighlighted(true);
	}
}

void InspectableContentComponent::mouseExit(const MouseEvent & e)
{
	//DBG("Mouse Exit : " << inspectable->getHelpID());
	String helpID = "";
	if (!inspectable.wasObjectDeleted())
	{
		helpID = inspectable->getHelpID();
		for (auto & i : inspectable->linkedInspectables)
		{
			if (!i.wasObjectDeleted()) i->setHighlighted(false);
		}
	}

	HelpBox::getInstance()->clearOverData(helpID);
}

void InspectableContentComponent::mouseDown(const MouseEvent & e)
{

	if (inspectable.wasObjectDeleted())
	{
		DBG("Object deleted on inspectable content, should never be there !");
		return;
	}

	if (!inspectable->isSelectable) return;
	if (!e.mods.isLeftButtonDown()) return;

	if (autoSelectWithChildRespect)
	{
		Component * c = e.eventComponent;

		bool foundAChildComponent = false;
		while (c != this)
		{
			InspectableContentComponent * ie = dynamic_cast<InspectableContentComponent *>(c);
			if (ie != nullptr)
			{
				foundAChildComponent = true;
				break;
			}
			c = c->getParentComponent();
		}

		if (!foundAChildComponent)
		{
			
			//grabKeyboardFocus();
			inspectable->selectThis(e.mods.isCommandDown() || e.mods.isShiftDown());
		}
	}
}

void InspectableContentComponent::paintOverChildren(Graphics & g)
{
	if (inspectable.wasObjectDeleted()) return;
	
	if (inspectable->isHighlighted)
	{
		g.setColour(highlightColor);
		g.drawRoundedRectangle(getMainBounds().toFloat(), rounderCornerSize, 4);
	}

	if (autoDrawContourWhenSelected && inspectable->isSelected)
	{
		g.setColour(selectionContourColor);
		g.drawRoundedRectangle(getMainBounds().toFloat(), rounderCornerSize, 2);
	}
}

Rectangle<int> InspectableContentComponent::getMainBounds()
{
	return getLocalBounds();

}
int InspectableContentComponent::getExtraWidth()
{
	return getWidth() - getMainBounds().getWidth();
}
int InspectableContentComponent::getExtraHeight()
{
	return getHeight() - getMainBounds().getHeight();
}
void InspectableContentComponent::newMessage(const Inspectable::InspectableEvent & e)
{
	if (e.type == Inspectable::InspectableEvent::SELECTION_CHANGED || e.type == Inspectable::InspectableEvent::PRESELECTION_CHANGED)
	{
		if (bringToFrontOnSelect) toFront(true);
		if (repaintOnSelectionChanged) repaint();
	}
	else if (e.type == Inspectable::InspectableEvent::HIGHLIGHT_CHANGED)
	{
		repaint();
	}
}
	
