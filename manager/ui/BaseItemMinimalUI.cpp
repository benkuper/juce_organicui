#include "JuceHeader.h"

BaseItemMinimalUI::BaseItemMinimalUI(BaseItem* item) :
	InspectableContentComponent(item),
	baseItem(item),
	bgColor(item != nullptr && item->itemColor != nullptr ? item->itemColor->getColor() : BG_COLOR.brighter(.1f)),
	selectedColor(HIGHLIGHT_COLOR),
	syncWithItemSize(false),
	viewZoom(1),
	viewCheckerSize(1),
	dimAlphaOnDisabled(true),
	highlightOnMouseOver(false),
	fillColorOnSelected(false),
	dragAndDropEnabled(true),
	autoHideWhenDragging(true),
	drawEmptyDragIcon(false),
	isDraggingOver(false),
	highlightOnDragOver(true)
{
	setDisableInternalMouseEvents(true);
	addMouseListener(this, true); //needs fixing, this is called twice on the component

	if (baseItem != nullptr)
	{
		setName(baseItem->niceName);
		baseItem->addAsyncContainerListener(this);
		if (baseItem->canBeDisabled && dimAlphaOnDisabled) setAlpha(baseItem->enabled->boolValue() ? 1 : .5f);
	}

}



BaseItemMinimalUI::~BaseItemMinimalUI()
{
	if (baseItem != nullptr && !inspectable.wasObjectDeleted()) baseItem->removeAsyncContainerListener(this);
}



void BaseItemMinimalUI::setHighlightOnMouseOver(bool highlight)
{
	setRepaintsOnMouseActivity(highlight);
	highlightOnMouseOver = highlight;
}


void BaseItemMinimalUI::paint(juce::Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	juce::Rectangle<float> r = this->getMainBounds().toFloat();
	bool isItemEnabled = baseItem->canBeDisabled ? baseItem->enabled->boolValue() : true;

	juce::Colour c = (fillColorOnSelected && baseItem->isSelected) ? selectedColor : bgColor;
	if (isItemEnabled) c = c.darker(.3f);
	if (highlightOnMouseOver && isMouseOverOrDragging(true)) c = c.brighter(.1f);
	g.setColour(c);
	g.fillRoundedRectangle(r, 4);

	if (isDraggingOver && highlightOnDragOver)
	{
		g.setColour(BLUE_COLOR);
		g.drawRoundedRectangle(r, 4, 2);
	}
}


void BaseItemMinimalUI::setViewZoom(float value)
{
	viewZoom = value;
}


void BaseItemMinimalUI::setViewCheckerSize(float value)
{
	viewCheckerSize = value;
	if (syncWithItemSize) setViewSize(baseItem->viewUISize->getPoint());
}



void BaseItemMinimalUI::setViewSize(float x, float y)
{
	setSize(x * viewCheckerSize, y * viewCheckerSize);
}


void BaseItemMinimalUI::setViewSize(juce::Point<float> size)
{
	setSize(size.x * viewCheckerSize, size.y * viewCheckerSize);
}


void BaseItemMinimalUI::updateItemUISize()
{
	setViewSize(baseItem->viewUISize->getPoint());
}


void BaseItemMinimalUI::mouseDown(const juce::MouseEvent& e)
{
	//LOG("BaseItemMinimalUI::mouseDown " << e.eventComponent->getName() << " / " << e.originalComponent->getName());
	InspectableContentComponent::mouseDown(e);

	if (e.mods.isRightButtonDown())
	{
		if (getMainBounds().contains(e.getPosition()))
		{
			juce::PopupMenu p;
			addContextMenuItems(p);

			if (p.getNumItems() == 0) return;

			p.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
				{
					if (result > 0) handleContextMenuResult(result);
				}
			);
		}
	}
	else
	{
		if (baseItem->isUILocked->boolValue()) return;
		baseItem->setMovePositionReference(true);
	}
}



void BaseItemMinimalUI::mouseDrag(const juce::MouseEvent& e)
{
	InspectableContentComponent::mouseDrag(e);

	if (e.mods.isLeftButtonDown())
	{
		if (!dragAndDropEnabled || isDragAndDropActive() || !canStartDrag(e)) return;
		if (baseItem->isUILocked->boolValue()) return;

		if (e.getDistanceFromDragStart() > dragStartDistance)
		{
			juce::Point<int> offset = getDragOffset();

			juce::var desc = juce::var(new juce::DynamicObject());
			desc.getDynamicObject()->setProperty("type", baseItem->getTypeString());
			desc.getDynamicObject()->setProperty("dataType", baseItem->itemDataType.isNotEmpty() ? baseItem->itemDataType : baseItem->getTypeString());
			desc.getDynamicObject()->setProperty("initX", baseItem->viewUIPosition->x);
			desc.getDynamicObject()->setProperty("initY", baseItem->viewUIPosition->y);
			desc.getDynamicObject()->setProperty("offsetX", offset.x);
			desc.getDynamicObject()->setProperty("offsetY", offset.y);

			juce::Image dragImage = drawEmptyDragIcon ? juce::Image(juce::Image::PixelFormat::ARGB, 1, 1, true) : juce::Image();// this->createComponentSnapshot(this->getLocalBounds()).convertedToFormat(Image::ARGB).rescaled(this->getWidth() * this->viewZoom, this->getHeight() * this->viewZoom);
			//dragImage.multiplyAllAlphas(drawEmptyDragIcon ? 0 : .5f);

			juce::Point<int> imageOffset = -offset;
			startDragging(desc, this, juce::ScaledImage(dragImage), true, &imageOffset);
		}
	}
}




void BaseItemMinimalUI::mouseExit(const juce::MouseEvent& e)
{
	InspectableContentComponent::mouseExit(e);
	repaint();
}


bool BaseItemMinimalUI::isUsingMouseWheel()
{
	return false;
}


void BaseItemMinimalUI::selectToThis()
{
	itemMinimalUIListeners.call(&ItemMinimalUIListener::askSelectToThis, this);
}


void BaseItemMinimalUI::newMessage(const ContainerAsyncEvent& e)
{
	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
	{
		if (e.targetControllable.wasObjectDeleted()) return;

		if (e.targetControllable == baseItem->enabled)
		{
			if (baseItem->canBeDisabled && dimAlphaOnDisabled) setAlpha(baseItem->enabled->boolValue() ? 1 : .5f);
			repaint();
		}
		else if (e.targetControllable == baseItem->viewUIPosition)
		{
			itemMinimalUIListeners.call(&ItemMinimalUIListener::itemUIViewPositionChanged, this);
		}
		else if (e.targetControllable == baseItem->viewUISize && syncWithItemSize)
		{
			setViewSize(baseItem->viewUISize->getPoint());
		}
		else if (e.targetControllable == baseItem->itemColor)
		{
			bgColor = baseItem->itemColor->getColor();
			repaint();
		}

		controllableFeedbackUpdateInternal(e.targetControllable);
	}
	break;

	case ContainerAsyncEvent::ControllableStateUpdate:
	{
		if (e.targetControllable.wasObjectDeleted()) return;
		controllableStateUpdateInternal(e.targetControllable);
	}
	break;

	case ContainerAsyncEvent::ChildAddressChanged:
	{
		if (e.targetContainer.wasObjectDeleted()) return;
		containerChildAddressChangedAsync(e.targetContainer);
	}
	break;

	default:
		break;
	}
}


void BaseItemMinimalUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == this->baseItem->viewUISize)
	{
		updateItemUISize();
	}
}


bool BaseItemMinimalUI::canStartDrag(const juce::MouseEvent& e)
{
	return e.eventComponent == this && !e.mods.isAltDown() && !e.mods.isCommandDown() && !e.mods.isShiftDown();
}


void BaseItemMinimalUI::dragOperationStarted(const juce::DragAndDropTarget::SourceDetails&)
{
	if (autoHideWhenDragging) setVisible(false);
}


void BaseItemMinimalUI::dragOperationEnded(const juce::DragAndDropTarget::SourceDetails&)
{
	if (autoHideWhenDragging) setVisible(true);
}


// Inherited via DragAndDropTarget

bool BaseItemMinimalUI::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	return acceptedDropTypes.contains(dragSourceDetails.description.getProperty("dataType", "").toString());
}



void BaseItemMinimalUI::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	if (highlightOnDragOver) repaint();
}


void BaseItemMinimalUI::itemDragExit(const SourceDetails&)
{
	isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}


void BaseItemMinimalUI::itemDropped(const SourceDetails& dragSourceDetails)
{
	isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}


juce::Point<int> BaseItemMinimalUI::getDragOffset()
{
	return getMouseXYRelative() * viewZoom;
}

bool BaseItemMinimalUI::isGroupUI()
{
	if (inspectable.wasObjectDeleted()) return false;
	return baseItem->isGroup;
}