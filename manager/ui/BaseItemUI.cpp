#include "JuceHeader.h"

BaseItemUI::BaseItemUI(BaseItem* _item, Direction _resizeDirection, bool showMiniModeBT) :
	BaseItemMinimalUI(_item),
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
	showGroupManager(false),
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


BaseItemUI::~BaseItemUI()
{
	if (removeBT != nullptr) removeBT->removeListener(this);
	if (GlobalSettings::getInstanceWithoutCreating() != nullptr) GlobalSettings::getInstance()->fontSize->removeAsyncParameterListener(this);
}

void BaseItemUI::setContentSize(int contentWidth, int contentHeight)
{
	int targetHeight = getHeightWithoutContent() + contentHeight + this->getExtraHeight();
	int targetWidth = contentWidth + margin * 2 + resizerWidth + this->getExtraWidth();

	this->setSize(targetWidth, targetHeight);
}

void BaseItemUI::setShowGroupManager(bool value)
{
	if (showGroupManager == value) return;
	showGroupManager = value;
	if (showGroupManager && isGroupUI())
	{
		groupManagerUI.reset(createGroupManagerUI());
		this->addAndMakeVisible(groupManagerUI.get());
		contentComponents.add(groupManagerUI.get());

		updateGroupManagerBounds();
	}
	else
	{
		if (groupManagerUI != nullptr)
		{
			contentComponents.removeAllInstancesOf(groupManagerUI.get());
			this->removeChildComponent(groupManagerUI.get());
		}
		groupManagerUI.reset();
	}
}

int BaseItemUI::getHeightWithoutContent()
{
	return headerHeight + margin * 2 + (this->baseItem->miniMode->boolValue() ? 0 : headerGap + resizerHeight);
}

void BaseItemUI::updateMiniModeUI()
{
	//auto hide/show component in content section
	for (auto& c : contentComponents) c->setVisible(!this->baseItem->miniMode->boolValue());

	updateItemUISize();

	itemUIListeners.call(&ItemUIListener::itemUIMiniModeChanged, this);
}

void BaseItemUI::updateItemUISize()
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



void BaseItemUI::resized()
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
			resizedContent(r);
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

			resizedContent(r);
		}
	}
}

void BaseItemUI::resizedHeader(juce::Rectangle<int>& r)
{
	int textWidth = (int)juce::TextLayout::getStringWidth(itemLabel.getFont(), this->baseItem->niceName);
	int labelWidth = juce::jmax(textWidth + 10, 30);
	itemLabel.setBounds(r.removeFromLeft(labelWidth).reduced(0, 1));
	r.removeFromLeft(2);

	resizedInternalHeader(r);
}

void BaseItemUI::resizedContent(juce::Rectangle<int>& r)
{
	if (showGroupManager && isGroupUI())
	{
		r.setHeight(jmax(groupManagerUI->headerSize, groupManagerUI->getHeight()));
		groupManagerUI->setBounds(r);
	}

	resizedInternalContent(r);
}

void BaseItemUI::updateGroupManagerBounds()
{
	if (inspectable.wasObjectDeleted() || baseItem->miniMode->boolValue() || !showGroupManager) return;
	int th = getHeightWithoutContent() + groupManagerUI->getHeight();
	if (th != getHeight())
	{
		baseItem->listUISize->setValue(th);
		resized();
	}
}

void BaseItemUI::buttonClicked(juce::Button* b)
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


void BaseItemUI::mouseDown(const juce::MouseEvent& e)
{
	//if ((removeBT != nullptr && e.eventComponent == removeBT) || (enabledBT != nullptr && e.eventComponent == enabledBT->bt)) return;
	if (dynamic_cast<juce::Button*>(e.eventComponent) != nullptr) return;
	else if (dynamic_cast<ControllableUI*>(e.eventComponent) != nullptr) return;
	else if (e.eventComponent == cornerResizer.get())
	{
		this->baseItem->setSizeReference(true);
	}

	if (e.eventComponent == this || e.eventComponent == &itemLabel) BaseItemMinimalUI::mouseDown(e);
}

void BaseItemUI::mouseDrag(const juce::MouseEvent& e)
{
	if (e.eventComponent == cornerResizer.get())
	{
		this->itemMinimalUIListeners.call(&BaseItemMinimalUI::ItemMinimalUIListener::itemUIResizeDrag, this, e.getOffsetFromDragStart());
	}

	BaseItemMinimalUI::mouseDrag(e);
}

void BaseItemUI::mouseUp(const juce::MouseEvent& e)
{
	if (e.eventComponent == cornerResizer.get())
	{
		this->itemMinimalUIListeners.call(&BaseItemMinimalUI::ItemMinimalUIListener::itemUIResizeEnd, this);
	}
}

void BaseItemUI::labelTextChanged(juce::Label* l)
{
	if (l == &itemLabel)
	{
		if (l->getText().isEmpty()) itemLabel.setText(this->baseItem->niceName, juce::dontSendNotification); //avoid setting empty names
		else this->baseItem->setUndoableNiceName(l->getText());
		resized();
	}
}

bool BaseItemUI::keyPressed(const juce::KeyPress& e)
{
	if (!this->inspectable.wasObjectDeleted() && this->baseItem->isSelected)
	{
		if (e.getKeyCode() == juce::KeyPress::F2Key)
		{
			this->itemLabel.showEditor();
			return true;
		}
	}

	return InspectableContentComponent::keyPressed(e);

}

bool BaseItemUI::canStartDrag(const juce::MouseEvent& e)
{
	return e.eventComponent == this || (e.eventComponent == &itemLabel && !itemLabel.isBeingEdited());
}


void BaseItemUI::containerChildAddressChangedAsync(ControllableContainer*)
{

	itemLabel.setText(this->baseItem->niceName, juce::dontSendNotification);
}

void BaseItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemMinimalUI::controllableFeedbackUpdateInternal(c);
	if (c == this->baseItem->miniMode) updateMiniModeUI();
}

void BaseItemUI::newMessage(const Parameter::ParameterEvent& e)
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

void BaseItemUI::childBoundsChanged(Component* c)
{
	updateGroupManagerBounds();
}

void BaseItemUI::visibilityChanged()
{
	resized();
}

void BaseItemUI::componentVisibilityChanged(juce::Component& c)
{
	if (&c == warningUI.get()) resized();
}


BaseManagerUI* BaseItemUI::createGroupManagerUI()
{
	BaseItemGroup* ig = dynamic_cast<BaseItemGroup*>(this->baseItem);
	BaseManager* m = ig->baseManager;
	return new BaseManagerUI(m->niceName, m);
}
