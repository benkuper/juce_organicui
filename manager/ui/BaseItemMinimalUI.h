/*
  ==============================================================================

	BaseItemMinimalUI.h
	Created: 20 Nov 2016 2:48:09pm
	Author:  Ben Kuper

  ==============================================================================
*/

#pragma once

template<class T>
class BaseItemMinimalUI :
	public InspectableContentComponent,
	public ContainerAsyncListener,
	public DragAndDropContainer,
	public DragAndDropTarget
{
public:
	BaseItemMinimalUI<T>(T* _item);
	virtual ~BaseItemMinimalUI<T>();

	T* item;
	BaseItem* baseItem;

	//ui
	Colour bgColor;
	Colour selectedColor;

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
	StringArray acceptedDropTypes;
	bool isDraggingOver;
	bool highlightOnDragOver;


	void setHighlightOnMouseOver(bool highlight);
	void paint(Graphics& g) override;
	void setViewZoom(float value);
	void setViewCheckerSize(float value);

	void setViewSize(float x, float y);
	void setViewSize(Point<float> size);

	virtual void updateItemUISize();

	virtual void mouseDown(const MouseEvent& e) override;
	virtual void mouseDrag(const MouseEvent& e) override;
	virtual void mouseExit(const MouseEvent& e) override;

	virtual bool isUsingMouseWheel();

	virtual void selectToThis() override;

	virtual void addContextMenuItems(PopupMenu& p) {}
	virtual void handleContextMenuResult(int result) {}

	virtual void newMessage(const ContainerAsyncEvent& e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer*) {}
	virtual void controllableFeedbackUpdateInternal(Controllable*);
	virtual void controllableStateUpdateInternal(Controllable*) {}

	//Drag drop container
	virtual bool canStartDrag(const MouseEvent& e);
	virtual void dragOperationStarted(const DragAndDropTarget::SourceDetails&) override;
	virtual void dragOperationEnded(const DragAndDropTarget::SourceDetails&) override;

	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails& dragSourceDetails) override; //to be overriden
	virtual Point<int> getDragOffset();

	class ItemMinimalUIListener
	{
	public:
		virtual ~ItemMinimalUIListener() {}

		virtual void itemUIViewPositionChanged(BaseItemMinimalUI<T>*) {}

		virtual void itemUIResizeDrag(BaseItemMinimalUI<T>*, const Point<int>& dragOffset) {}
		virtual void itemUIResizeEnd(BaseItemMinimalUI<T>*) {}

		virtual void askForSyncPosAndSize(BaseItemMinimalUI<T>*) {}
		virtual void askSelectToThis(BaseItemMinimalUI<T>*) {}
	};

	ListenerList<ItemMinimalUIListener> itemMinimalUIListeners;
	void addItemMinimalUIListener(ItemMinimalUIListener* newListener) { itemMinimalUIListeners.add(newListener); }
	void removeItemMinimalUIListener(ItemMinimalUIListener* listener) { itemMinimalUIListeners.remove(listener); }
};


template<class T>
BaseItemMinimalUI<T>::BaseItemMinimalUI(T* _item) :
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

	setDisableDefaultMouseEvents(true);
	addMouseListener(this, true); //needs fixing, this is called twice on the component

	if (baseItem != nullptr)
	{
		baseItem->addAsyncContainerListener(this);
		if (baseItem->canBeDisabled && dimAlphaOnDisabled) setAlpha(baseItem->enabled->boolValue() ? 1 : .5f);
	}

}


template<class T>
BaseItemMinimalUI<T>::~BaseItemMinimalUI()
{
	if (baseItem != nullptr) baseItem->removeAsyncContainerListener(this);
}


template<class T>
void BaseItemMinimalUI<T>::setHighlightOnMouseOver(bool highlight)
{
	setRepaintsOnMouseActivity(highlight);
	highlightOnMouseOver = highlight;
}

template<class T>
void BaseItemMinimalUI<T>::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	juce::Rectangle<float> r = this->getMainBounds().toFloat();
	bool isItemEnabled = baseItem->canBeDisabled ? baseItem->enabled->boolValue() : true;

	Colour c = (fillColorOnSelected && baseItem->isSelected) ? selectedColor : bgColor;
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
void BaseItemMinimalUI<T>::setViewZoom(float value)
{
	viewZoom = value;
}

template<class T>
void BaseItemMinimalUI<T>::setViewCheckerSize(float value)
{
	viewCheckerSize = value;
	if (syncWithItemSize) setViewSize(item->viewUISize->getPoint());
}


template<class T>
void BaseItemMinimalUI<T>::setViewSize(float x, float y)
{
	setSize(x * viewCheckerSize, y * viewCheckerSize);
}

template<class T>
void BaseItemMinimalUI<T>::setViewSize(Point<float> size)
{
	setSize(size.x * viewCheckerSize, size.y * viewCheckerSize);
}

template<class T>
void BaseItemMinimalUI<T>::updateItemUISize()
{
	setViewSize(baseItem->viewUISize->getPoint());
}

template<class T>
void BaseItemMinimalUI<T>::mouseDown(const MouseEvent& e)
{
	InspectableContentComponent::mouseDown(e);

	if (e.mods.isRightButtonDown())
	{
		PopupMenu p;
		addContextMenuItems(p);

		if (p.getNumItems() == 0) return;

		p.showMenuAsync(PopupMenu::Options(), [this](int result)
			{
				if (result > 0) handleContextMenuResult(result);
			}
		);


	}
	else
	{
		if (item->isUILocked->boolValue()) return;
		baseItem->setMovePositionReference(true);
	}
}


template<class T>
void BaseItemMinimalUI<T>::mouseDrag(const MouseEvent& e)
{
	InspectableContentComponent::mouseDrag(e);

	if (e.mods.isLeftButtonDown())
	{
		if (!dragAndDropEnabled || isDragAndDropActive() || !canStartDrag(e)) return;
		if (item->isUILocked->boolValue()) return;

		if (e.getDistanceFromDragStart() > dragStartDistance)
		{
			Point<int> offset = getDragOffset();

			var desc = var(new DynamicObject());
			desc.getDynamicObject()->setProperty("type", baseItem->getTypeString());
			desc.getDynamicObject()->setProperty("dataType", baseItem->itemDataType.isNotEmpty()?baseItem->itemDataType:baseItem->getTypeString());
			desc.getDynamicObject()->setProperty("initX", baseItem->viewUIPosition->x);
			desc.getDynamicObject()->setProperty("initY", baseItem->viewUIPosition->y);
			desc.getDynamicObject()->setProperty("offsetX", offset.x);
			desc.getDynamicObject()->setProperty("offsetY", offset.y);

			Image dragImage = drawEmptyDragIcon ? Image(Image::PixelFormat::ARGB, 1, 1, true) : Image();// this->createComponentSnapshot(this->getLocalBounds()).convertedToFormat(Image::ARGB).rescaled(this->getWidth() * this->viewZoom, this->getHeight() * this->viewZoom);
			//dragImage.multiplyAllAlphas(drawEmptyDragIcon ? 0 : .5f);

			Point<int> imageOffset = -offset;
			startDragging(desc, this, ScaledImage(dragImage), true, &imageOffset);
		}
	}
}



template<class T>
void BaseItemMinimalUI<T>::mouseExit(const MouseEvent& e)
{
	InspectableContentComponent::mouseExit(e);
	repaint();
}

template<class T>
bool BaseItemMinimalUI<T>::isUsingMouseWheel()
{
	return false;
}

template<class T>
void BaseItemMinimalUI<T>::selectToThis()
{
	itemMinimalUIListeners.call(&ItemMinimalUIListener::askSelectToThis, this);
}

template<class T>
void BaseItemMinimalUI<T>::newMessage(const ContainerAsyncEvent& e)
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
void BaseItemMinimalUI<T>::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == this->baseItem->viewUISize)
	{
		updateItemUISize();
	}
}

template<class T>
bool BaseItemMinimalUI<T>::canStartDrag(const MouseEvent& e)
{
	return e.eventComponent == this && !e.mods.isAltDown() && !e.mods.isCommandDown() && !e.mods.isShiftDown();
}

template<class T>
void BaseItemMinimalUI<T>::dragOperationStarted(const DragAndDropTarget::SourceDetails&)
{
	if (autoHideWhenDragging) setVisible(false);
}

template<class T>
void BaseItemMinimalUI<T>::dragOperationEnded(const DragAndDropTarget::SourceDetails&)
{
	if (autoHideWhenDragging) setVisible(true);
}


// Inherited via DragAndDropTarget
template<class T>
bool BaseItemMinimalUI<T>::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	return acceptedDropTypes.contains(dragSourceDetails.description.getProperty("dataType", "").toString());
}


template<class T>
void BaseItemMinimalUI<T>::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	if (highlightOnDragOver) repaint();
}

template<class T>
void BaseItemMinimalUI<T>::itemDragExit(const SourceDetails&)
{
	isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}

template<class T>
void BaseItemMinimalUI<T>::itemDropped(const SourceDetails& dragSourceDetails)
{
	isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}

template<class T>
Point<int> BaseItemMinimalUI<T>::getDragOffset()
{
	return getMouseXYRelative() * viewZoom;
}
