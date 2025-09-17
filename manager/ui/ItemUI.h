/*
  ==============================================================================

	ItemUI.h
	Created: 28 Oct 2016 8:04:09pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class T>
class ItemUI :
	public ItemMinimalUI<T>,
	public juce::Button::Listener,
	public juce::Label::Listener,
	public juce::ComponentListener,
	public Parameter::AsyncListener
{
public:
	enum Direction { NONE, VERTICAL, HORIZONTAL, ALL };

	ItemUI(T* _item, Direction resizeDirection = NONE, bool showMiniModeBT = false);
	virtual ~ItemUI();

	//LAYOUT
	int margin;
	int minContentHeight;


	juce::Point<float> sizeAtMouseDown;

	//header
	int headerHeight;
	int headerGap;
	bool showEnableBT;
	bool showRemoveBT;
	bool showColorUI;

	//Resize
	Direction resizeDirection;
	int resizerWidth;
	int resizerHeight;


	class ItemResizerComponent :
		public juce::Component
	{
	public:
		ItemResizerComponent()
		{
			setMouseCursor(juce::MouseCursor::BottomRightCornerResizeCursor);
		}

		~ItemResizerComponent() {}

		void paint(juce::Graphics& g)
		{
			g.setColour(isMouseOverOrDragging() ? HIGHLIGHT_COLOR : juce::Colours::lightgrey.withAlpha(.3f));
			for (int i = 0; i < 3; i++) g.drawLine(getWidth() * i / 3.f, (float)getHeight(), (float)getWidth(), getHeight() * i / 3.f);
		}
	};

	std::unique_ptr<ItemResizerComponent> cornerResizer;
	std::unique_ptr<juce::ResizableEdgeComponent> edgeResizer;
	juce::ComponentBoundsConstrainer constrainer;

	juce::Component* resizer;

	juce::Label itemLabel;
	std::unique_ptr<BoolToggleUI> enabledBT;
	std::unique_ptr<juce::ImageButton> removeBT;
	std::unique_ptr<ColorParameterUI> itemColorUI;
	std::unique_ptr<WarningTargetUI> warningUI;
	std::unique_ptr<BoolToggleUI> miniModeBT;

	//std::unique_ptr<Grabber> grabber;

	juce::Array<juce::Component*> contentComponents;

	void setContentSize(int contentWidth, int contentHeight);

	//minimode
	int getHeightWithoutContent();

	virtual void updateMiniModeUI();
	virtual void updateItemUISize() override;

	//void setGrabber(Grabber * newGrabber);

	virtual void resized() override;
	virtual void resizedHeader(juce::Rectangle<int>& r);
	virtual void resizedInternalHeader(juce::Rectangle<int>&) {}
	virtual void resizedInternalContent(juce::Rectangle<int>&) {}
	virtual void resizedInternalFooter(juce::Rectangle<int>&) {}
	virtual void buttonClicked(juce::Button* b) override;

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseDrag(const juce::MouseEvent& e) override;
	virtual void mouseUp(const juce::MouseEvent& e) override;

	virtual void labelTextChanged(juce::Label* l) override;
	virtual bool keyPressed(const juce::KeyPress& e) override;

	virtual bool canStartDrag(const juce::MouseEvent& e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer*) override;
	virtual void controllableFeedbackUpdateInternal(Controllable*) override;

	virtual void newMessage(const Parameter::ParameterEvent& e) override;

	virtual void visibilityChanged() override;

	virtual void componentVisibilityChanged(juce::Component& c) override;

	class ItemUIListener
	{
	public:
		virtual ~ItemUIListener() {}
		virtual void itemUIMiniModeChanged(ItemUI<T>*) {}
	};

	juce::ListenerList<ItemUIListener> ItemUIListeners;
	void addItemUIListener(ItemUIListener* newListener) { ItemUIListeners.add(newListener); }
	void removeItemUIListener(ItemUIListener* listener) { ItemUIListeners.remove(listener); }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemUI)
};


template<class T>
ItemUI<T>::ItemUI(T* _item, Direction _resizeDirection, bool showMiniModeBT) :
	ItemMinimalUI<T>(_item),
	margin(3),
	minContentHeight(2),
	headerHeight(GlobalSettings::getInstance()->fontSize->floatValue() + 2),
	headerGap(2),
	showEnableBT(true),
	showRemoveBT(true),
	showColorUI(true),
	resizeDirection(_resizeDirection),
	resizerWidth(0),
	resizerHeight(0),
	resizer(nullptr),
	itemLabel("itemLabel", dynamic_cast<BaseItem*>(this->inspectable.get())->niceName)
{

	this->setName(this->baseItem->niceName);

	itemLabel.setColour(itemLabel.backgroundColourId, juce::Colours::transparentWhite);
	itemLabel.setColour(itemLabel.textColourId, TEXT_COLOR);

	itemLabel.setColour(itemLabel.backgroundWhenEditingColourId, juce::Colours::black);
	itemLabel.setColour(itemLabel.textWhenEditingColourId, juce::Colours::white);
	itemLabel.setColour(juce::CaretComponent::caretColourId, juce::Colours::orange);

	itemLabel.setFont(juce::FontOptions(GlobalSettings::getInstance()->fontSize->floatValue()));
	itemLabel.setJustificationType(juce::Justification::centredLeft);

	itemLabel.setEditable(false, this->baseItem->nameCanBeChangedByUser);
	itemLabel.addListener(this);
	this->addAndMakeVisible(&itemLabel);

	switch (resizeDirection)
	{
	case NONE:
		break;

	case VERTICAL:
		resizerHeight = 8;
		constrainer.setMinimumHeight(headerHeight + headerGap + minContentHeight + resizerHeight);
		edgeResizer.reset(new juce::ResizableEdgeComponent(this, &constrainer, juce::ResizableEdgeComponent::bottomEdge));
		edgeResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(edgeResizer.get());
		break;

	case HORIZONTAL:
		resizerWidth = 4;
		constrainer.setMinimumWidth(20 + resizerWidth); // ??
		edgeResizer.reset(new juce::ResizableEdgeComponent(this, &constrainer, juce::ResizableEdgeComponent::rightEdge));
		edgeResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(edgeResizer.get());
		break;

	case ALL:
		resizerHeight = 10;
		cornerResizer.reset(new ItemResizerComponent());
		cornerResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(cornerResizer.get());
		break;
	}

	if (this->baseItem->canBeDisabled)
	{
		enabledBT.reset(this->baseItem->enabled->createToggle(juce::ImageCache::getFromMemory(OrganicUIBinaryData::power_png, OrganicUIBinaryData::power_pngSize)));
		this->addAndMakeVisible(enabledBT.get());
	}

	if (this->baseItem->userCanRemove)
	{
		removeBT.reset(AssetManager::getInstance()->getRemoveBT());
		removeBT->setWantsKeyboardFocus(false);
		removeBT->setMouseClickGrabsKeyboardFocus(false);
		this->addAndMakeVisible(removeBT.get());
		removeBT->addListener(this);
	}

	if (this->baseItem->itemColor != nullptr)
	{
		itemColorUI.reset(this->baseItem->itemColor->createColorParamUI());
		this->addAndMakeVisible(itemColorUI.get());
	}

	if (this->baseItem->showWarningInUI)
	{
		warningUI.reset(new WarningTargetUI(this->baseItem));
		warningUI->addComponentListener(this);
		this->addChildComponent(warningUI.get());
	}

	if (showMiniModeBT)
	{
		miniModeBT.reset(this->baseItem->miniMode->createToggle(juce::ImageCache::getFromMemory(OrganicUIBinaryData::minus_png, OrganicUIBinaryData::minus_pngSize)));
		this->addAndMakeVisible(miniModeBT.get());
	}

	this->setHighlightOnMouseOver(true);

	GlobalSettings::getInstance()->fontSize->addAsyncParameterListener(this);

	updateMiniModeUI();
}


template<class T>
ItemUI<T>::~ItemUI()
{
	if (removeBT != nullptr) removeBT->removeListener(this);
	if (GlobalSettings::getInstanceWithoutCreating() != nullptr) GlobalSettings::getInstance()->fontSize->removeAsyncParameterListener(this);
}

template<class T>
void ItemUI<T>::setContentSize(int contentWidth, int contentHeight)
{
	int targetHeight = getHeightWithoutContent() + contentHeight + this->getExtraHeight();
	int targetWidth = contentWidth + margin * 2 + resizerWidth + this->getExtraWidth();

	this->setSize(targetWidth, targetHeight);
}

template<class T>
int ItemUI<T>::getHeightWithoutContent()
{
	return headerHeight + margin * 2 + (this->baseItem->miniMode->boolValue() ? 0 : headerGap + resizerHeight);
}

template<class T>
void ItemUI<T>::updateMiniModeUI()
{
	//auto hide/show component in content section
	for (auto& c : contentComponents) c->setVisible(!this->baseItem->miniMode->boolValue());

	updateItemUISize();

	ItemUIListeners.call(&ItemUIListener::itemUIMiniModeChanged, this);
}

template<class T>
void ItemUI<T>::updateItemUISize()
{
	if (this->baseItem->miniMode->boolValue())
	{
		if (resizer != nullptr) this->removeChildComponent(resizer);

		int targetWidth = this->getWidth();
		if (targetWidth == 0)
		{
			if (resizeDirection == ALL) targetWidth = this->baseItem->viewUISize->x;
			else if (resizeDirection == HORIZONTAL) targetWidth = this->baseItem->listUISize->floatValue();
		}

		int targetHeight = getHeightWithoutContent();
		if (targetHeight == 0)
		{
			if (resizeDirection == ALL) targetHeight = this->baseItem->viewUISize->y;
			else if (resizeDirection == VERTICAL) targetHeight = this->baseItem->listUISize->floatValue();
		}

		this->setSize(targetWidth, targetHeight);
	}
	else
	{
		if (resizer != nullptr) this->addAndMakeVisible(resizer);

		int targetHeight = this->getHeight() > 0 ? juce::jmax(minContentHeight, this->getHeight()) : 24;//Default size if zero
		int targetWidth = this->getWidth() > 0 ? this->getWidth() : 100;//default size if zero

		switch (resizeDirection)
		{
		case ALL:
			targetWidth = (int)this->baseItem->viewUISize->x;
			targetHeight = (int)this->baseItem->viewUISize->y;
			break;

		case VERTICAL:
			targetHeight = (int)this->baseItem->listUISize->floatValue();
			break;

		case HORIZONTAL:
			targetWidth = (int)this->baseItem->listUISize->floatValue();
			break;

		case NONE:
			targetWidth = (int)this->baseItem->listUISize->floatValue();
			targetHeight = (int)this->baseItem->listUISize->floatValue();
			break;
		}

		this->setViewSize(targetWidth, targetHeight);
	}

}



template<class T>
void ItemUI<T>::resized()
{
	if (!this->isVisible()) return;

	//Header
	if (this->getWidth() == 0 || this->getHeight() == 0) return;
	juce::Rectangle<int> r = this->getMainBounds().reduced(margin);

	//Grabber

	juce::Rectangle<int> h = r.removeFromTop(headerHeight);


	if (enabledBT != nullptr && showEnableBT)
	{
		enabledBT->setBounds(h.removeFromLeft(h.getHeight()));
		h.removeFromLeft(2);
	}

	if (warningUI != nullptr && warningUI->isVisible())
	{
		warningUI->setBounds(h.removeFromLeft(h.getHeight())); //warning
		h.removeFromLeft(2);
	}

	if (removeBT != nullptr && showRemoveBT)
	{
		removeBT->setBounds(h.removeFromRight(h.getHeight()));
		h.removeFromRight(2);
	}

	if (itemColorUI != nullptr && showColorUI)
	{
		itemColorUI->setBounds(h.removeFromRight(h.getHeight()).reduced(2));
		h.removeFromRight(2);
	}

	if (miniModeBT != nullptr)
	{
		miniModeBT->setBounds(h.removeFromRight(h.getHeight()));
		h.removeFromRight(2);
	}

	resizedHeader(h);

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
				this->baseItem->listUISize->setValue(this->getHeight());

			}
			break;

			case HORIZONTAL:
			{
				edgeResizer->setBounds(r.removeFromRight(resizerWidth));
				this->baseItem->listUISize->setValue(this->getWidth());
			}
			break;

			case ALL:
			{
				juce::Rectangle<int> fr = r.removeFromBottom(resizerHeight);
				resizedInternalFooter(fr);
				cornerResizer->setBounds(fr.withLeft(r.getWidth() - resizerHeight));

				//this->baseItem->viewUISize->setPoint(this->getWidth(), this->getHeight());

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
void ItemUI<T>::resizedHeader(juce::Rectangle<int>& r)
{
	int textWidth = (int)juce::TextLayout::getStringWidth(itemLabel.getFont(), this->baseItem->niceName);
	int labelWidth = juce::jmax(textWidth + 10, 30);
	itemLabel.setBounds(r.removeFromLeft(labelWidth).reduced(0, 1));
	r.removeFromLeft(2);

	resizedInternalHeader(r);
}

template<class T>
void ItemUI<T>::buttonClicked(juce::Button* b)
{
	if (b == removeBT.get())
	{
		if (this->baseItem->askConfirmationBeforeRemove && GlobalSettings::getInstance()->askBeforeRemovingItems->boolValue())
		{
			juce::AlertWindow::showAsync(
				juce::MessageBoxOptions().withIconType(juce::AlertWindow::QuestionIcon)
				.withTitle("Delete " + this->baseItem->niceName)
				.withMessage("Are you sure you want to delete this ?")
				.withButton("Delete")
				.withButton("Cancel"),
				[this](int result)
				{
					if (result != 0) this->baseItem->remove();
				}
			);
		}
		else this->baseItem->remove();
	}
}


template<class T>
void ItemUI<T>::mouseDown(const juce::MouseEvent& e)
{
	//if ((removeBT != nullptr && e.eventComponent == removeBT) || (enabledBT != nullptr && e.eventComponent == enabledBT->bt)) return;
	if (dynamic_cast<juce::Button*>(e.eventComponent) != nullptr) return;
	else if (dynamic_cast<ControllableUI*>(e.eventComponent) != nullptr) return;
	else if (e.eventComponent == cornerResizer.get())
	{
		this->baseItem->setSizeReference(true);
	}

	if (e.eventComponent == this || e.eventComponent == &itemLabel) ItemMinimalUI<T>::mouseDown(e);
}

template<class T>
void ItemUI<T>::mouseDrag(const juce::MouseEvent& e)
{
	if (e.eventComponent == cornerResizer.get())
	{
		this->itemMinimalUIListeners.call(&ItemMinimalUI<T>::ItemMinimalUIListener::itemUIResizeDrag, this, e.getOffsetFromDragStart());
	}

	ItemMinimalUI<T>::mouseDrag(e);
}

template<class T>
void ItemUI<T>::mouseUp(const juce::MouseEvent& e)
{
	if (e.eventComponent == cornerResizer.get())
	{
		this->itemMinimalUIListeners.call(&ItemMinimalUI<T>::ItemMinimalUIListener::itemUIResizeEnd, this);
	}
}

template<class T>
void ItemUI<T>::labelTextChanged(juce::Label* l)
{
	if (l == &itemLabel)
	{
		if (l->getText().isEmpty()) itemLabel.setText(this->baseItem->niceName, juce::dontSendNotification); //avoid setting empty names
		else this->baseItem->setUndoableNiceName(l->getText());
		resized();
	}
}

template<class T>
bool ItemUI<T>::keyPressed(const juce::KeyPress& e)
{
	if (!this->inspectable.wasObjectDeleted() && this->item->isSelected)
	{
		if (e.getKeyCode() == juce::KeyPress::F2Key)
		{
			this->itemLabel.showEditor();
			return true;
		}
	}

	return InspectableContentComponent::keyPressed(e);

}

template<class T>
bool ItemUI<T>::canStartDrag(const juce::MouseEvent& e)
{
	return e.eventComponent == this || (e.eventComponent == &itemLabel && !itemLabel.isBeingEdited());
}


template<class T>
void ItemUI<T>::containerChildAddressChangedAsync(ControllableContainer*)
{

	itemLabel.setText(this->baseItem->niceName, juce::dontSendNotification);
}

template<class T>
void ItemUI<T>::controllableFeedbackUpdateInternal(Controllable* c)
{
	ItemMinimalUI<T>::controllableFeedbackUpdateInternal(c);
	if (c == this->baseItem->miniMode) updateMiniModeUI();
}

template<class T>
void ItemUI<T>::newMessage(const Parameter::ParameterEvent& e)
{
	if (e.type == e.VALUE_CHANGED && e.parameter == GlobalSettings::getInstance()->fontSize)
	{
		if (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing) return;

		bool isDefaultHeight = headerHeight == itemLabel.getFont().getHeight() + 2;
		itemLabel.setFont(juce::FontOptions(GlobalSettings::getInstance()->fontSize->floatValue()));
		if (isDefaultHeight) headerHeight = GlobalSettings::getInstance()->fontSize->floatValue() + 2;
		resized();
	}
}

template<class T>
void ItemUI<T>::visibilityChanged()
{
	resized();
}

template<class T>
void ItemUI<T>::componentVisibilityChanged(juce::Component& c)
{
	if (&c == warningUI.get()) resized();
}
