/*
  ==============================================================================

	InspectableContentComponent.cpp
	Created: 9 May 2016 6:51:16pm
	Author:  bkupe

  ==============================================================================
*/

InspectableContentComponent::InspectableContentComponent(Inspectable * _inspectable) :
	InspectableContent(nullptr), //force set in constructor
	repaintOnSelectionChanged(true),
	rounderCornerSize(4),
	autoDrawContourWhenSelected(true),
	highlightLinkedInspectablesOnOver(true),
    selectionContourColor(HIGHLIGHT_COLOR),
    autoSelectWithChildRespect(true),
    highlightColor(Colours::purple),
    bringToFrontOnSelect(true)
{
	//setWantsKeyboardFocus(true);
	setMouseClickGrabsKeyboardFocus(true);
	setInspectable(_inspectable); //set here to have setInspectable run
}

InspectableContentComponent::~InspectableContentComponent()
{
	if (!inspectable.wasObjectDeleted())
	{
		inspectable->removeAsyncInspectableListener(this);
		inspectable->setSelected(false);
	}
}


void InspectableContentComponent::setInspectable(Inspectable* i)
{
	if (inspectable != nullptr && !inspectable.wasObjectDeleted())
	{
		inspectable->removeAsyncInspectableListener(this);
	}

	InspectableContent::setInspectable(i);

	if (inspectable != nullptr)
	{
		inspectable->addAsyncInspectableListener(this);
	}
}

void InspectableContentComponent::mouseEnter(const MouseEvent & e)
{
	//DBG("Mouse Enter : " << inspectable->getHelpID());
	if (inspectable.wasObjectDeleted()) return;
	if (HelpBox::getInstance()->overDataID.isEmpty()) HelpBox::getInstance()->setOverData(inspectable->getHelpID());

	if (highlightLinkedInspectablesOnOver) inspectable->highlightLinkedInspectables(true);
}

void InspectableContentComponent::mouseExit(const MouseEvent & e)
{
	//DBG("Mouse Exit : " << inspectable->getHelpID());
	String helpID = "";
	if (!inspectable.wasObjectDeleted())
	{
		helpID = inspectable->getHelpID();
		if (highlightLinkedInspectablesOnOver) inspectable->highlightLinkedInspectables(false);

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
		while (c != this && c != nullptr)
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
			if (e.mods.isShiftDown()) inspectable->selectThis(true);
			else
			{
				if (!inspectable->isSelected) inspectable->selectThis(e.mods.isCommandDown());
				else if(e.mods.isCommandDown()) inspectable->deselectThis();
			}
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

	if (autoDrawContourWhenSelected && (inspectable->isSelected || inspectable->isPreselected))
	{
		g.setColour(inspectable->isSelected?selectionContourColor:PRESELECT_COLOR);
		g.drawRoundedRectangle(getMainBounds().toFloat(), rounderCornerSize, 2);
	}
}

juce::Rectangle<int> InspectableContentComponent::getMainBounds()
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
		if (bringToFrontOnSelect && !inspectable.wasObjectDeleted() && inspectable->isSelected) toFront(false);
		if (repaintOnSelectionChanged) repaint();
	}
	else if (e.type == Inspectable::InspectableEvent::HIGHLIGHT_CHANGED)
	{
		repaint();
	}
}
	
