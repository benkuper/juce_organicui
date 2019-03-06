/*
  ==============================================================================

	BaseItemUI.h
	Created: 28 Oct 2016 8:04:09pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class T>
class BaseItemUI :
	public BaseItemMinimalUI<T>,
	public Button::Listener,
	public Label::Listener
{
public:
	enum Direction { NONE, VERTICAL, HORIZONTAL, ALL };

	BaseItemUI<T>(T * _item, Direction resizeDirection = NONE, Direction dragDirection = VERTICAL);
	virtual ~BaseItemUI<T>();

	//LAYOUT
	int margin;
	int minContentHeight;

	float viewZoom;

	//grabber
	int grabberHeight;
	Point<float> posAtMouseDown;
	Point<float> sizeAtMouseDown;
	Direction dragDirection;
	bool customDragBehavior;

	//header
	int headerHeight;
	int headerGap;
	bool showEnableBT;
	bool showRemoveBT;

	//Resize
	Direction resizeDirection;
	int resizerWidth;
	int resizerHeight;

	//list pos
	Point<int> posAtDown;
	Point<int> dragOffset; //for list grabbing

	ScopedPointer<ResizableCornerComponent> cornerResizer;
	ScopedPointer<ResizableEdgeComponent> edgeResizer;
	ComponentBoundsConstrainer constrainer;

	Component * resizer;

	Label itemLabel;
	ScopedPointer<BoolImageToggleUI> enabledBT;
	ScopedPointer<ImageButton> removeBT;

	ScopedPointer<Grabber> grabber;

	Array<Component *> contentComponents;

	void setContentSize(int contentWidth, int contentHeight);

	//minimode
	int getHeightWithoutContent();

	virtual void updateMiniModeUI();

	void setViewZoom(float value);
	void setGrabber(Grabber * newGrabber);

	virtual void resized() override;
	virtual void resizedInternalHeader(juce::Rectangle<int> &) {}
	virtual void resizedInternalContent(juce::Rectangle<int> &) {}
	virtual void resizedInternalFooter(juce::Rectangle<int> &) {}
	void buttonClicked(Button *b) override;


	void mouseDown(const MouseEvent &e) override;
	void mouseDrag(const MouseEvent &e) override;
	void mouseUp(const MouseEvent &e) override;
	void mouseDoubleClick(const MouseEvent &e) override;
	virtual void labelTextChanged(Label * l) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer *) override;
	virtual void controllableFeedbackUpdateInternal(Controllable *) override;

	virtual void visibilityChanged() override;



	class ItemUIListener
	{
	public:
		virtual ~ItemUIListener() {}
		virtual void itemUIMiniModeChanged(BaseItemUI<T> *) {}
		virtual void itemUIGrabStart(BaseItemUI<T> *) {}
		virtual void itemUIGrabbed(BaseItemUI<T> *) {}
		virtual void itemUIGrabEnd(BaseItemUI<T> *) {}
	};

	ListenerList<ItemUIListener> itemUIListeners;
	void addItemUIListener(ItemUIListener* newListener) { itemUIListeners.add(newListener); }
	void removeItemUIListener(ItemUIListener* listener) { itemUIListeners.remove(listener); }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseItemUI<T>)
};


template<class T>
BaseItemUI<T>::BaseItemUI(T * _item, Direction _resizeDirection, Direction _dragDirection) :
	BaseItemMinimalUI<T>(_item),
	margin(3),
	minContentHeight(2),
	viewZoom(1),
	grabberHeight(0),
	dragDirection (_dragDirection),
	customDragBehavior(false),
	headerHeight(16),
	headerGap(2),
	showEnableBT(true),
	showRemoveBT(true),
	resizeDirection(_resizeDirection),
	resizerWidth(0),
	resizerHeight(0),
	resizer(nullptr),
	itemLabel("itemLabel", dynamic_cast<BaseItem *>(this->inspectable.get())->niceName)
{
	itemLabel.setColour(itemLabel.backgroundColourId, Colours::transparentWhite);
	itemLabel.setColour(itemLabel.textColourId, TEXT_COLOR);

	itemLabel.setColour(itemLabel.backgroundWhenEditingColourId, Colours::black);
	itemLabel.setColour(itemLabel.textWhenEditingColourId, Colours::white);
	itemLabel.setColour(CaretComponent::caretColourId, Colours::orange);

	itemLabel.setFont((float)(headerHeight - 4));
	itemLabel.setJustificationType(Justification::centredLeft);

	itemLabel.setEditable(false, this->baseItem->nameCanBeChangedByUser);
	itemLabel.addListener(this);
	this->addAndMakeVisible(&itemLabel);

	if (dragDirection != NONE)
	{
		setGrabber(new Grabber(dragDirection == VERTICAL ? Grabber::VERTICAL : Grabber::HORIZONTAL));
	}

	switch (resizeDirection)
	{
	case NONE:
		break;

	case VERTICAL:
		resizerHeight = 8;
		constrainer.setMinimumHeight(headerHeight + headerGap + minContentHeight + resizerHeight);
		edgeResizer = new ResizableEdgeComponent(this, &constrainer, ResizableEdgeComponent::bottomEdge);
		edgeResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(edgeResizer);
		//resizer = edgeResizer;
		//setContentSize(getWidth(),(int)item->listUISize->floatValue());
		break;

	case HORIZONTAL:
		resizerWidth = 4;
		constrainer.setMinimumWidth(20 + resizerWidth); // ??
		edgeResizer = new ResizableEdgeComponent(this, &constrainer, ResizableEdgeComponent::rightEdge);
		edgeResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(edgeResizer);
		//resizer = edgeResizer;
		//setContentSize((int)item->listUISize->floatValue(),getHeight());
		break;

	case ALL:
		resizerHeight = 10;
		constrainer.setMinimumSize(resizerWidth + 20, headerHeight + headerGap + minContentHeight + resizerHeight);
		cornerResizer = new ResizableCornerComponent(this, &constrainer);
		cornerResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(cornerResizer);
		//resizer = cornerResizer;
		//setContentSize((int)item->viewUISize->getPoint().x, (int)item->viewUISize->getPoint().y);
		break;
	}

	if (this->baseItem->canBeDisabled)
	{
		enabledBT = this->baseItem->enabled->createImageToggle(AssetManager::getInstance()->getPowerBT());
		this->addAndMakeVisible(enabledBT);
	}

	if (this->baseItem->userCanRemove)
	{
		removeBT = AssetManager::getInstance()->getRemoveBT();
		this->addAndMakeVisible(removeBT);
		removeBT->addListener(this);
	}

	this->setHighlightOnMouseOver(true);

	updateMiniModeUI();
}

template<class T>
BaseItemUI<T>::~BaseItemUI()
{
	if (removeBT != nullptr) removeBT->removeListener(this);
}

template<class T>
void BaseItemUI<T>::setContentSize(int contentWidth, int contentHeight)
{
	int targetHeight = getHeightWithoutContent() + contentHeight + this->getExtraHeight();
	int targetWidth = contentWidth + margin * 2 + resizerWidth + this->getExtraWidth();

	this->setSize(targetWidth, targetHeight);
}

template<class T>
int BaseItemUI<T>::getHeightWithoutContent()
{
	return grabberHeight + headerHeight + margin * 2 + (this->baseItem->miniMode->boolValue() ? 0 : headerGap + resizerHeight);
}

template<class T>
void BaseItemUI<T>::updateMiniModeUI()
{
	//auto hide/show component in content section
	for (auto &c : contentComponents) c->setVisible(!this->baseItem->miniMode->boolValue());

	if (this->baseItem->miniMode->boolValue())
	{
		if (resizer != nullptr) this->removeChildComponent(resizer);

		int targetHeight = getHeightWithoutContent();
		this->setSize((int)this->baseItem->viewUISize->getPoint().x, targetHeight);
	}
	else
	{
		if (resizer != nullptr) this->addAndMakeVisible(resizer);

		int targetHeight = 0;
		int targetWidth = this->getWidth();

		switch (resizeDirection)
		{
		case ALL:
			targetWidth = (int)this->baseItem->viewUISize->getPoint().x;
			targetHeight = (int)this->baseItem->viewUISize->getPoint().y;
			break;
		case VERTICAL:
			targetHeight = (int)this->baseItem->listUISize->floatValue();
			break;
		case HORIZONTAL:
			targetWidth = (int)this->baseItem->listUISize->floatValue();
			break;
		case NONE:
			break;
		}

		setContentSize(targetWidth, targetHeight);
	}

	itemUIListeners.call(&ItemUIListener::itemUIMiniModeChanged, this);
}

template<class T>
void BaseItemUI<T>::setViewZoom(float value)
{
	viewZoom = value;
}



template<class T>
void BaseItemUI<T>::resized()
{
	if (!this->isVisible()) return;

	//Header
	if (this->getWidth() == 0 || this->getHeight() == 0) return;
	juce::Rectangle<int> r = this->getMainBounds().reduced(margin);

	//Grabber
	if (dragDirection == HORIZONTAL || dragDirection == ALL)
	{
		//Grabber
		grabber->setBounds(r.removeFromTop(grabberHeight));
		grabber->repaint();
	}

	juce::Rectangle<int> h = r.removeFromTop(headerHeight);

	if (dragDirection == VERTICAL)
	{
		grabber->setBounds(h.removeFromLeft(10));
		grabber->repaint();
	}

	if (enabledBT != nullptr && showEnableBT)
	{
		enabledBT->setBounds(h.removeFromLeft(h.getHeight()));
		h.removeFromLeft(2);
	}

	if (removeBT != nullptr && showRemoveBT) removeBT->setBounds(h.removeFromRight(h.getHeight()));
	h.removeFromRight(2);

	resizedInternalHeader(h);
	itemLabel.setBounds(h.reduced(1));

	r.removeFromTop(headerGap);

	if (!this->baseItem->miniMode->boolValue())
	{
		if (resizeDirection == NONE)
		{
			int top = r.getY();
			resizedInternalContent(r);
			if (r.getWidth() == 0 || r.getHeight() == 0) return;
			setContentSize(r.getWidth(), r.getBottom() - top);
		}
		else
		{
			switch (resizeDirection)
			{
			case VERTICAL:
			{
				juce::Rectangle<int> fr = r.removeFromBottom(resizerHeight);
				resizedInternalFooter(fr);
				edgeResizer->setBounds(fr);
				this->baseItem->listUISize->setValue((float)r.getHeight());

			}
			break;

			case HORIZONTAL:
			{
				edgeResizer->setBounds(r.removeFromRight(resizerWidth));
				this->baseItem->listUISize->setValue((float)r.getWidth());
			}
			break;

			case ALL:
			{
				juce::Rectangle<int> fr = r.removeFromBottom(resizerHeight);
				resizedInternalFooter(fr);
				cornerResizer->setBounds(fr.withLeft(r.getWidth() - resizerHeight));
				this->baseItem->viewUISize->setPoint((float)r.getWidth() + this->getExtraWidth(), (float)r.getHeight() + this->getExtraHeight());
			}
			break;

			default:
				break;

			}
			resizedInternalContent(r);
		}
	}
}

template<class T>
void BaseItemUI<T>::buttonClicked(Button * b)
{
	if (b == removeBT)
	{
		if (this->baseItem->askConfirmationBeforeRemove && GlobalSettings::getInstance()->askBeforeRemovingItems->boolValue())
		{
			int result = AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon, "Delete " + this->baseItem->niceName, "Are you sure you want to delete this ?", "Delete", "Cancel");
			if (result != 0)this->baseItem->remove();
		}
		else this->baseItem->remove();
	}
}


template<class T>
void BaseItemUI<T>::mouseDown(const MouseEvent & e)
{
	if ((removeBT != nullptr && e.eventComponent == removeBT) || (enabledBT != nullptr && e.eventComponent == enabledBT->bt)) return;

	BaseItemMinimalUI<T>::mouseDown(e);

	if (e.mods.isLeftButtonDown())
	{
		if (e.eventComponent == cornerResizer.get())
		{
			sizeAtMouseDown = this->baseItem->viewUISize->getPoint();

		}else if (dragDirection != NONE && !customDragBehavior  && e.eventComponent->getParentComponent() == this)
		{
			Grabber * g = dynamic_cast<Grabber *>(e.eventComponent);
			if (g != nullptr)
			{
				if (dragDirection  == ALL || dragDirection == NONE)
				{
					posAtMouseDown = this->baseItem->viewUIPosition->getPoint();
				}
				else
				{
					posAtDown = this->getBounds().getPosition();
					dragOffset = Point<int>();
					itemUIListeners.call(&ItemUIListener::itemUIGrabStart, this);
				}
			}
		}
	}
}


template<class T>
void BaseItemUI<T>::mouseDrag(const MouseEvent & e)
{
	BaseItemMinimalUI<T>::mouseDrag(e);

	if (e.mods.isLeftButtonDown())
	{
		if (dragDirection != NONE && !customDragBehavior && e.eventComponent->getParentComponent() == this)
		{
			Grabber * g = dynamic_cast<Grabber *>(e.eventComponent);
			if (g != nullptr)
			{
				if (dragDirection == ALL || dragDirection == NONE)
				{
					Point<float> targetPos = posAtMouseDown + e.getOffsetFromDragStart().toFloat() / viewZoom;
					this->baseItem->viewUIPosition->setPoint(targetPos);
				}
				else
				{
					dragOffset = e.getOffsetFromDragStart();
					itemUIListeners.call(&ItemUIListener::itemUIGrabbed, this);
				}
			}
		}

	}

}

template<class T>
void BaseItemUI<T>::mouseUp(const MouseEvent & e)
{
	if (e.mods.isLeftButtonDown())
	{
		if (e.eventComponent == cornerResizer.get())
		{
			Point<float> sizeDiffDemi = ((this->baseItem->viewUISize->getPoint() - sizeAtMouseDown) / 2).toFloat();
			this->baseItem->viewUIPosition->setPoint(this->baseItem->viewUIPosition->getPoint() + sizeDiffDemi);
			this->baseItem->viewUISize->setPoint(this->baseItem->viewUISize->getPoint());

		}else if (dragDirection != NONE && !customDragBehavior  && e.eventComponent->getParentComponent() == this)
		{
			Grabber * g = dynamic_cast<Grabber *>(e.eventComponent);
			if (g != nullptr)
			{
				if (dragDirection == ALL || dragDirection == NONE)
				{
					this->baseItem->viewUIPosition->setUndoablePoint(posAtMouseDown, this->baseItem->viewUIPosition->getPoint());
				}
				else
				{
					itemUIListeners.call(&ItemUIListener::itemUIGrabEnd, this);
				}
			}
		}

	}
}

template<class T>
void BaseItemUI<T>::mouseDoubleClick(const MouseEvent & e)
{
	if (e.eventComponent == grabber) this->baseItem->miniMode->setValue(!this->baseItem->miniMode->boolValue());
}

template<class T>
void BaseItemUI<T>::labelTextChanged(Label * l)
{
	if (l == &itemLabel)
	{
		if (l->getText().isEmpty()) itemLabel.setText(this->baseItem->niceName, dontSendNotification); //avoid setting empty names
		else this->baseItem->setUndoableNiceName(l->getText());
	}
}


template<class T>
void BaseItemUI<T>::containerChildAddressChangedAsync(ControllableContainer *)
{

	itemLabel.setText(this->baseItem->niceName, dontSendNotification);
}

template<class T>
void BaseItemUI<T>::controllableFeedbackUpdateInternal(Controllable * c)
{
	BaseItemMinimalUI<T>::controllableFeedbackUpdateInternal(c);
	if (c == this->baseItem->miniMode) updateMiniModeUI();
	else if (dragDirection != NONE && c == this->baseItem->viewUIPosition) itemUIListeners.call(&ItemUIListener::itemUIGrabbed, this);
}

template<class T>
void BaseItemUI<T>::visibilityChanged()
{
	resized();
}

template<class T>
void BaseItemUI<T>::setGrabber(Grabber * newGrabber)
{
	if (grabber != nullptr)
	{
		this->removeChildComponent(grabber);
	}

	grabber = newGrabber;

	if (grabber != nullptr)
	{
		this->addAndMakeVisible(grabber);
		if (dragDirection == HORIZONTAL ||dragDirection == ALL) grabberHeight = 15;
		this->addAndMakeVisible(grabber);
	}

}