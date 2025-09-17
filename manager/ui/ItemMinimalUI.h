/*
  ==============================================================================

	ItemMinimalUI.h
	Created: 20 Nov 2016 2:48:09pm
	Author:  Ben Kuper

  ==============================================================================
*/

#pragma once

template<class T>
class ItemMinimalUI :
	public InspectableContentComponent,
	public ContainerAsyncListener,
	public juce::DragAndDropContainer,
	public juce::DragAndDropTarget
{
public:
	ItemMinimalUI(T* _item);
	virtual ~ItemMinimalUI();

	T* item;
	BaseItem* baseItem;

	//ui
	juce::Colour bgColor;
	juce::Colour selectedColor;

	bool syncWithItemSize;

	float viewZoom;
	float viewCheckerSize;

	bool dimAlphaOnDisabled;
	bool highlightOnMouseOver;
	bool fillColorOnSelected;

	//Dragging
	bool dragAndDropEnabled; //tmp, waiting to implement full drag&drop system
	int dragStartDistance = 10;
	bool autoHideWhenDragging;
	bool drawEmptyDragIcon;

	//dropping
	juce::StringArray acceptedDropTypes;
	bool isDraggingOver;
	bool highlightOnDragOver;


	void setHighlightOnMouseOver(bool highlight);
	void paint(juce::Graphics& g) override;
	void setViewZoom(float value);
	void setViewCheckerSize(float value);

	void setViewSize(float x, float y);
	void setViewSize(juce::Point<float> size);

	virtual void updateItemUISize();

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseDrag(const juce::MouseEvent& e) override;
	virtual void mouseExit(const juce::MouseEvent& e) override;

	virtual bool isUsingMouseWheel();

	virtual void selectToThis() override;

	virtual void addContextMenuItems(juce::PopupMenu& p) {}
	virtual void handleContextMenuResult(int result) {}

	virtual void newMessage(const ContainerAsyncEvent& e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer*) {}
	virtual void controllableFeedbackUpdateInternal(Controllable*);
	virtual void controllableStateUpdateInternal(Controllable*) {}

	//Drag drop container
	virtual bool canStartDrag(const juce::MouseEvent& e);
	virtual void dragOperationStarted(const juce::DragAndDropTarget::SourceDetails&) override;
	virtual void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails&) override;

	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails& dragSourceDetails) override; //to be overriden
	virtual juce::Point<int> getDragOffset();

	class ItemMinimalUIListener
	{
	public:
		virtual ~ItemMinimalUIListener() {}

		virtual void itemUIViewPositionChanged(ItemMinimalUI<T>*) {}

		virtual void itemUIResizeDrag(ItemMinimalUI<T>*, const juce::Point<int>& dragOffset) {}
		virtual void itemUIResizeEnd(ItemMinimalUI<T>*) {}

		virtual void askForSyncPosAndSize(ItemMinimalUI<T>*) {}
		virtual void askSelectToThis(ItemMinimalUI<T>*) {}
	};

	juce::ListenerList<ItemMinimalUIListener> itemMinimalUIListeners;
	void addItemUIMinimalUIListener(ItemMinimalUIListener* newListener) { itemMinimalUIListeners.add(newListener); }
	void removeItemMinimalUIListener(ItemMinimalUIListener* listener) { itemMinimalUIListeners.remove(listener); }
};


template<class T>
ItemMinimalUI<T>::ItemMinimalUI(T* _item) :
	InspectableContentComponent(_item),
	item(_item),
	bgColor(_item != nullptr && _item->itemColor != nullptr ? _item->itemColor->getColor() : BG_COLOR.brighter(.1f)),
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
	baseItem = static_cast<BaseItem*>(item);

	setDisableInternalMouseEvents(true);
	addMouseListener(this, true); //needs fixing, this is called twice on the component

	if (baseItem != nullptr)
	{
		setName(baseItem->niceName);
		baseItem->addAsyncContainerListener(this);
		if (baseItem->canBeDisabled && dimAlphaOnDisabled) setAlpha(baseItem->enabled->boolValue() ? 1 : .5f);
	}

}


template<class T>
ItemMinimalUI<T>::~ItemMinimalUI()
{
	if (baseItem != nullptr && !inspectable.wasObjectDeleted()) baseItem->removeAsyncContainerListener(this);
}


template<class T>
void ItemMinimalUI<T>::setHighlightOnMouseOver(bool highlight)
{
	setRepaintsOnMouseActivity(highlight);
	highlightOnMouseOver = highlight;
}

template<class T>
void ItemMinimalUI<T>::paint(juce::Graphics& g)
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

template<class T>
void ItemMinimalUI<T>::setViewZoom(float value)
{
	viewZoom = value;
}

template<class T>
void ItemMinimalUI<T>::setViewCheckerSize(float value)
{
	viewCheckerSize = value;
	if (syncWithItemSize) setViewSize(item->viewUISize->getPoint());
}


template<class T>
void ItemMinimalUI<T>::setViewSize(float x, float y)
{
	setSize(x * viewCheckerSize, y * viewCheckerSize);
}

template<class T>
void ItemMinimalUI<T>::setViewSize(juce::Point<float> size)
{
	setSize(size.x * viewCheckerSize, size.y * viewCheckerSize);
}

template<class T>
void ItemMinimalUI<T>::updateItemUISize()
{
	setViewSize(baseItem->viewUISize->getPoint());
}

template<class T>
void ItemMinimalUI<T>::mouseDown(const juce::MouseEvent& e)
{
	//LOG("ItemMinimalUI::mouseDown " << e.eventComponent->getName() << " / " << e.originalComponent->getName());
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
		if (item->isUILocked->boolValue()) return;
		baseItem->setMovePositionReference(true);
	}
}


template<class T>
void ItemMinimalUI<T>::mouseDrag(const juce::MouseEvent& e)
{
	InspectableContentComponent::mouseDrag(e);

	if (e.mods.isLeftButtonDown())
	{
		if (!dragAndDropEnabled || isDragAndDropActive() || !canStartDrag(e)) return;
		if (item->isUILocked->boolValue()) return;

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



template<class T>
void ItemMinimalUI<T>::mouseExit(const juce::MouseEvent& e)
{
	InspectableContentComponent::mouseExit(e);
	repaint();
}

template<class T>
bool ItemMinimalUI<T>::isUsingMouseWheel()
{
	return false;
}

template<class T>
void ItemMinimalUI<T>::selectToThis()
{
	itemMinimalUIListeners.call(&ItemMinimalUIListener::askSelectToThis, this);
}

template<class T>
void ItemMinimalUI<T>::newMessage(const ContainerAsyncEvent& e)
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
			setViewSize(item->viewUISize->getPoint());
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

template<class T>
void ItemMinimalUI<T>::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == this->baseItem->viewUISize)
	{
		updateItemUISize();
	}
}

template<class T>
bool ItemMinimalUI<T>::canStartDrag(const juce::MouseEvent& e)
{
	return e.eventComponent == this && !e.mods.isAltDown() && !e.mods.isCommandDown() && !e.mods.isShiftDown();
}

template<class T>
void ItemMinimalUI<T>::dragOperationStarted(const juce::DragAndDropTarget::SourceDetails&)
{
	if (autoHideWhenDragging) setVisible(false);
}

template<class T>
void ItemMinimalUI<T>::dragOperationEnded(const juce::DragAndDropTarget::SourceDetails&)
{
	if (autoHideWhenDragging) setVisible(true);
}


// Inherited via DragAndDropTarget
template<class T>
bool ItemMinimalUI<T>::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	return acceptedDropTypes.contains(dragSourceDetails.description.getProperty("dataType", "").toString());
}


template<class T>
void ItemMinimalUI<T>::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	if (highlightOnDragOver) repaint();
}

template<class T>
void ItemMinimalUI<T>::itemDragExit(const SourceDetails&)
{
	isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}

template<class T>
void ItemMinimalUI<T>::itemDropped(const SourceDetails& dragSourceDetails)
{
	isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}

template<class T>
juce::Point<int> ItemMinimalUI<T>::getDragOffset()
{
	return getMouseXYRelative() * viewZoom;
}
