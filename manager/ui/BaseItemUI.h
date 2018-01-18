/*
  ==============================================================================

	BaseItemUI.h
	Created: 28 Oct 2016 8:04:09pm
	Author:  bkupe

  ==============================================================================
*/

#ifndef BASEITEMUI_H_INCLUDED
#define BASEITEMUI_H_INCLUDED


template<class T>
class BaseItemUI :
	public BaseItemMinimalUI<T>,
	public ButtonListener,
	public LabelListener
{
public:
	enum ResizeMode { NONE, VERTICAL, HORIZONTAL, ALL };

	BaseItemUI<T>(T * _item, ResizeMode resizeMode = NONE, bool canBeDragged = true);
	virtual ~BaseItemUI<T>();
	
	//LAYOUT
    int margin;
    int minContentHeight;

	float viewZoom;
    
	//grabber
	int grabberHeight;
	Point<float> posAtMouseDown;
    bool canBeDragged;

	//header
	int headerHeight;
	int headerGap;
    
    //Resize
    ResizeMode resizeMode;
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

	void setContentSize(int contentWidth, int contentHeight);

	//minimode
	int getHeightWithoutContent();

	virtual void updateMiniModeUI();

	void setViewZoom(float value);

	virtual void resized() override;
	virtual void resizedInternalHeader(juce::Rectangle<int> &) {}
	virtual void resizedInternalContent(juce::Rectangle<int> &) {}
	void buttonClicked(Button *b) override;

	void mouseDown(const MouseEvent &e) override;
	void mouseDrag(const MouseEvent &e) override;
	void mouseUp(const MouseEvent &e) override;
	void mouseDoubleClick(const MouseEvent &e) override;
	virtual void labelTextChanged(Label * l) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer *) override;
	virtual void controllableFeedbackUpdateInternal(Controllable *) override;

	class Grabber : public Component
	{
	public:
		enum Direction { VERTICAL, HORIZONTAL };
		Grabber(Direction d = HORIZONTAL) : dir(d) {}
		~Grabber() {}

		Direction dir;

		void paint(Graphics &g) override;
	};

	void setGrabber(Grabber * newGrabber);
	ScopedPointer<Grabber> grabber;

	class ItemUIListener
	{
	public:
		virtual ~ItemUIListener() {}
		virtual void itemUIGrabStart(BaseItemUI<T> *) {}
		virtual void itemUIGrabbed(BaseItemUI<T> *) {}
		virtual void itemUIGrabEnd(BaseItemUI<T> *) {}
		virtual void itemUIMiniModeChanged(BaseItemUI<T> *) {}
	};

	ListenerList<ItemUIListener> itemUIListeners;
	void addItemUIListener(ItemUIListener* newListener) { itemUIListeners.add(newListener); }
	void removeItemUIListener(ItemUIListener* listener) { itemUIListeners.remove(listener); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseItemUI<T>)
};


template<class T>
BaseItemUI<T>::BaseItemUI(T * _item, ResizeMode _resizeMode, bool _canBeDragged) :
	BaseItemMinimalUI<T>(_item),
	margin(3),
	minContentHeight(2),
	viewZoom(1),
	grabberHeight(0),
	canBeDragged(_canBeDragged),
	headerHeight(16),
	headerGap(2),
	resizeMode(_resizeMode),
	resizerWidth(0),
	resizerHeight(0),
	resizer(nullptr),
	itemLabel("itemLabel", dynamic_cast<BaseItem *>(inspectable.get())->niceName)
{
	itemLabel.setColour(itemLabel.backgroundWhenEditingColourId, Colours::white);
	itemLabel.setColour(itemLabel.backgroundColourId, Colours::transparentWhite);
	itemLabel.setColour(itemLabel.textColourId, TEXT_COLOR);
	itemLabel.setFont((float)(headerHeight - 4));
	itemLabel.setJustificationType(Justification::centredLeft);

	itemLabel.setEditable(false,this->baseItem->nameCanBeChangedByUser);
	itemLabel.addListener(this);
	this->addAndMakeVisible(&itemLabel);

	if (canBeDragged)
	{
		setGrabber(new Grabber(_resizeMode == ALL ? Grabber::HORIZONTAL : Grabber::VERTICAL));
	}

	switch (resizeMode)
	{
	case NONE:
		break;

	case VERTICAL:
		resizerHeight = 4;
		constrainer.setMinimumHeight(headerHeight + headerGap + minContentHeight + resizerHeight);
		edgeResizer = new ResizableEdgeComponent(this, &constrainer, ResizableEdgeComponent::bottomEdge);
		edgeResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(edgeResizer);
		//resizer = edgeResizer;
		//setContentSize(getWidth(),(int)item->listUISize->floatValue());
		break;

	case HORIZONTAL:
		resizerWidth = 4;
		constrainer.setMinimumWidth(20+resizerWidth); // ??
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
	if(removeBT != nullptr) removeBT->removeListener(this);
}

template<class T>
void BaseItemUI<T>::setContentSize(int contentWidth, int contentHeight)
{
	int targetHeight = getHeightWithoutContent() + contentHeight;

	int targetWidth = contentWidth + margin * 2 + resizerWidth;

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
	if (this->baseItem->miniMode->boolValue())
	{
		if (resizer != nullptr) this->removeChildComponent(resizer);

		int targetHeight = getHeightWithoutContent();
		this->setSize((int)this->baseItem->viewUISize->getPoint().x, targetHeight);
	} else
	{
		if (resizer != nullptr) this->addAndMakeVisible(resizer);

		int targetHeight = 0;
		int targetWidth = this->getWidth();

		switch (resizeMode)
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
	DBG("Set view zoom : " << value);
	viewZoom = value;
}

template<class T>
void BaseItemUI<T>::resized()
{
	/*
	if (this->getHeight() < getHeightWithoutContent() + minContentHeight)
	{
		this->setSize(this->getWidth(), getHeightWithoutContent() + minContentHeight);
		return;
	}
	*/

	//Header
	if (this->getWidth() == 0 || this->getHeight() == 0) return;
	juce::Rectangle<int> r = this->getLocalBounds().reduced(margin);

	//Grabber
	if (canBeDragged)
	{
		//Grabber
		grabber->setBounds(r.removeFromTop(grabberHeight));
		grabber->repaint();
	}

	juce::Rectangle<int> h = r.removeFromTop(headerHeight);

	if (canBeDragged && resizeMode != ALL)
	{
		grabber->setBounds(h.removeFromLeft(10));
		grabber->repaint();
	}

	if (enabledBT != nullptr)
	{
		enabledBT->setBounds(h.removeFromLeft(h.getHeight()));
		h.removeFromLeft(2);
	}

	if(removeBT != nullptr) removeBT->setBounds(h.removeFromRight(h.getHeight()));
	h.removeFromRight(2);

	resizedInternalHeader(h);
	itemLabel.setBounds(h.reduced(1));

	r.removeFromTop(headerGap);

	if (!this->baseItem->miniMode->boolValue())
	{
		if (resizeMode == NONE)
		{
			int top = r.getY();
			resizedInternalContent(r);
			if (r.getWidth() == 0 || r.getHeight() == 0) return;
			setContentSize(r.getWidth(),r.getBottom() - top);
		} else
		{
			switch (resizeMode)
			{
			case VERTICAL:
				edgeResizer->setBounds(r.removeFromBottom(resizerHeight));
				this->baseItem->listUISize->setValue((float)r.getHeight());
				break;

			case HORIZONTAL:
				edgeResizer->setBounds(r.removeFromRight(resizerWidth));
				this->baseItem->listUISize->setValue((float)r.getWidth());
				break;

			case ALL:
				cornerResizer->setBounds(r.removeFromBottom(resizerHeight).withLeft(r.getWidth() - resizerHeight));
				this->baseItem->viewUISize->setPoint((float)r.getWidth(), (float)r.getHeight());
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
			int result = AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon, "Delete " + this->baseItem->niceName, "Are you sure you want to delete this ?","Delete","Cancel");
			if (result != 0)this->baseItem->remove();
		} else this->baseItem->remove();
	}
}


template<class T>
void BaseItemUI<T>::mouseDown(const MouseEvent & e)
{
	BaseItemMinimalUI<T>::mouseDown(e);


	if (e.mods.isLeftButtonDown())
	{
		if (canBeDragged && e.eventComponent == grabber)
		{
			if (resizeMode == ALL)
			{
				posAtMouseDown = this->baseItem->viewUIPosition->getPoint();
			} else
			{
				posAtDown = this->getBounds().getPosition();
				dragOffset = Point<int>();
				itemUIListeners.call(&ItemUIListener::itemUIGrabStart, this);
			}
		}
	}


}


template<class T>
void BaseItemUI<T>::mouseDrag(const MouseEvent & e)
{
	BaseItemMinimalUI<T>::mouseDrag(e);

	if (canBeDragged && e.mods.isLeftButtonDown() && e.eventComponent == grabber)
	{

		if (resizeMode == ALL)
		{
			Point<float> targetPos = posAtMouseDown + e.getOffsetFromDragStart().toFloat()/viewZoom;
			this->baseItem->viewUIPosition->setPoint(targetPos);
		} else
		{
			dragOffset = e.getOffsetFromDragStart();
			itemUIListeners.call(&ItemUIListener::itemUIGrabbed, this);
		}
	}

}

template<class T>
void BaseItemUI<T>::mouseUp(const MouseEvent &)
{
	if (canBeDragged)
	{
		if (resizeMode == ALL)
		{
			this->baseItem->viewUIPosition->setUndoablePoint(posAtMouseDown, this->baseItem->viewUIPosition->getPoint());
		} else
		{
			itemUIListeners.call(&ItemUIListener::itemUIGrabEnd, this);
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
	if (l == &itemLabel) this->baseItem->setUndoableNiceName(l->getText());
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
	else if (canBeDragged && c == this->baseItem->viewUIPosition) itemUIListeners.call(&ItemUIListener::itemUIGrabbed, this);
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
		if (resizeMode == ALL) grabberHeight = 15;
		this->addAndMakeVisible(grabber);
	}
	
}


template<class T>
void BaseItemUI<T>::Grabber::paint(Graphics & g)
{
	juce::Rectangle<int> r = getLocalBounds();
	g.setColour(BG_COLOR.brighter(.3f));
	const int numLines = 3;
	for (int i = 0; i < numLines; i++)
	{
		if (dir == HORIZONTAL)
		{
			float th = (i + 1)*(float)getHeight() / ((float)numLines + 1);
			g.drawLine(0, th, (float)getWidth(), th, 1);
		} else
		{
			float tw = (i + 1)*(float)getWidth() / ((float)numLines + 1);
			g.drawLine(tw, 0, tw, (float)getHeight(), 1);
		}

	}
}





#endif  // BASEITEMUI_H_INCLUDED
