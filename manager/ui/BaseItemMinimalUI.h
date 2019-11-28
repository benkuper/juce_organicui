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
	BaseItemMinimalUI<T>(T * _item);
	virtual ~BaseItemMinimalUI<T>();

	T * item;
	BaseItem * baseItem;

	//ui
	Colour bgColor;
	Colour selectedColor;

	float viewZoom;

	bool dimAlphaOnDisabled;
	bool highlightOnMouseOver;
	bool fillColorOnSelected;

	//Dragging
	bool dragAndDropEnabled; //tmp, waiting to implement full drag&drop system
	const int dragStartDistance = 10;
	bool autoHideWhenDragging;
	bool drawEmptyDragIcon;

	//dropping
	StringArray acceptedDropTypes;
	bool isDraggingOver;
	bool highlightOnDragOver;

	virtual void mouseDrag(const MouseEvent &e) override;
	virtual void mouseExit(const MouseEvent &e) override;


	void setHighlightOnMouseOver(bool highlight);

	void paint(Graphics &g) override;

	void setViewZoom(float value);

	virtual void newMessage(const ContainerAsyncEvent &e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer *) {}
	virtual void controllableFeedbackUpdateInternal(Controllable*);
	virtual void controllableStateUpdateInternal(Controllable*) {}

	//Drag drop container
	virtual bool canStartDrag(const MouseEvent &e);
	virtual void dragOperationStarted(const DragAndDropTarget::SourceDetails&) override;
	virtual void dragOperationEnded(const DragAndDropTarget::SourceDetails&) override;

	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails & dragSourceDetails) override;
	
	virtual void itemDragEnter(const SourceDetails& ) override;
	virtual void itemDragExit(const SourceDetails& ) override;
	virtual void itemDropped(const SourceDetails & dragSourceDetails) override; //to be overriden


	class ItemMinimalUIListener
	{
	public:
		virtual ~ItemMinimalUIListener() {}
		virtual void itemUIViewPositionChanged(BaseItemMinimalUI<T>*) {}
		virtual void askForSyncPosAndSize(BaseItemMinimalUI<T>*) {}
	};

	ListenerList<ItemMinimalUIListener> itemMinimalUIListeners;
	void addItemMinimalUIListener(ItemMinimalUIListener* newListener) { itemMinimalUIListeners.add(newListener); }
	void removeItemMinimalUIListener(ItemMinimalUIListener* listener) { itemMinimalUIListeners.remove(listener); }
};


template<class T>
BaseItemMinimalUI<T>::BaseItemMinimalUI(T* _item) :
	InspectableContentComponent(_item),
	item(_item),
	bgColor(BG_COLOR.brighter(.1f)),
	selectedColor(HIGHLIGHT_COLOR),
	viewZoom(1),
	dimAlphaOnDisabled(true),
	highlightOnMouseOver(false),
	fillColorOnSelected(false),
	dragAndDropEnabled(true),
	autoHideWhenDragging(true),
	drawEmptyDragIcon(false),
	isDraggingOver(false),
	highlightOnDragOver(true)
{
    baseItem = static_cast<BaseItem *>(item);

	setDisableDefaultMouseEvents(true);
	addMouseListener(this, true); //needs fixing, this is called twice on the component
	baseItem->addAsyncContainerListener(this);
	
	if (baseItem->canBeDisabled && dimAlphaOnDisabled) setAlpha(baseItem->enabled->boolValue() ? 1 : .5f);

}

template<class T>
BaseItemMinimalUI<T>::~BaseItemMinimalUI()
{
	baseItem->removeAsyncContainerListener(this);
}

template<class T>
void BaseItemMinimalUI<T>::mouseDrag(const MouseEvent & e)
{
	InspectableContentComponent::mouseDrag(e);

	if (e.mods.isLeftButtonDown())
	{
		if (!dragAndDropEnabled || isDragAndDropActive() || !canStartDrag(e)) return;

		var desc = var(new DynamicObject());
		desc.getDynamicObject()->setProperty("type", baseItem->getTypeString());
		desc.getDynamicObject()->setProperty("dataType", baseItem->itemDataType);
		desc.getDynamicObject()->setProperty("initX", baseItem->viewUIPosition->x);
		desc.getDynamicObject()->setProperty("initY", baseItem->viewUIPosition->y);
		desc.getDynamicObject()->setProperty("offsetX", (int)(getMouseXYRelative().x * viewZoom));
		desc.getDynamicObject()->setProperty("offsetY", (int)(getMouseXYRelative().y * viewZoom));

		Image dragImage = this->createComponentSnapshot(this->getLocalBounds()).convertedToFormat(Image::ARGB).rescaled(this->getWidth() * this->viewZoom, this->getHeight() * this->viewZoom);
		dragImage.multiplyAllAlphas(drawEmptyDragIcon ? 0 : .5f);

		Point<int> offset = -getMouseXYRelative() * viewZoom;
		if (e.getDistanceFromDragStart() > dragStartDistance) startDragging(desc, this, dragImage, true, &offset);
	}
}
	


template<class T>
void BaseItemMinimalUI<T>::mouseExit(const MouseEvent &e)
{
	InspectableContentComponent::mouseExit(e);
	repaint();
}

template<class T>
void BaseItemMinimalUI<T>::setHighlightOnMouseOver(bool highlight)
{
	setRepaintsOnMouseActivity(highlight);
	highlightOnMouseOver = highlight;
}

template<class T>
void BaseItemMinimalUI<T>::paint(Graphics &g)
{
	juce::Rectangle<float> r = this->getMainBounds().toFloat();
	bool isItemEnabled = baseItem->canBeDisabled ? baseItem->enabled->boolValue() : true;

	Colour c  = (fillColorOnSelected && baseItem->isSelected) ? selectedColor : bgColor;
	if(isItemEnabled) c = c.darker(.3f);
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
void BaseItemMinimalUI<T>::newMessage(const ContainerAsyncEvent & e)
{
	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
	{
		if (e.targetControllable == baseItem->enabled)
		{
			if (baseItem->canBeDisabled && dimAlphaOnDisabled) setAlpha(baseItem->enabled->boolValue() ? 1 : .5f);
			repaint();
		}else if(e.targetControllable  == baseItem->viewUIPosition)
		{
			itemMinimalUIListeners.call(&ItemMinimalUIListener::itemUIViewPositionChanged, this);
		}

		controllableFeedbackUpdateInternal(e.targetControllable);
	}
	break;

	case ContainerAsyncEvent::ControllableStateUpdate:
	{
		controllableStateUpdateInternal(e.targetControllable);
	}
	break;

	case ContainerAsyncEvent::ChildAddressChanged:
	{
		containerChildAddressChangedAsync(e.targetContainer);
	}
	break;

    default:
        break;
	}
}

template<class T>
void BaseItemMinimalUI<T>::controllableFeedbackUpdateInternal(Controllable * c)
{
	
}

template<class T>
bool BaseItemMinimalUI<T>::canStartDrag(const MouseEvent & e)
{
	return e.eventComponent == this;
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
bool BaseItemMinimalUI<T>::isInterestedInDragSource(const SourceDetails & dragSourceDetails)
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
void BaseItemMinimalUI<T>::itemDropped(const SourceDetails & dragSourceDetails)
{
	isDraggingOver = false;
	if(highlightOnDragOver) repaint();
}
