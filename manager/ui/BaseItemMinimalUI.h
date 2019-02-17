/*
  ==============================================================================

    BaseItemMinimalUI.h
    Created: 20 Nov 2016 2:48:09pm
    Author:  Ben Kuper

  ==============================================================================
*/

#ifndef BASEITEMMINIMALUI_H_INCLUDED
#define BASEITEMMINIMALUI_H_INCLUDED

template<class T>
class BaseItemMinimalUI :
	public InspectableContentComponent,
	public ContainerAsyncListener
{
public:
	BaseItemMinimalUI<T>(T * _item);
	virtual ~BaseItemMinimalUI<T>();

	T * item;
    BaseItem * baseItem;
    
	//ui
	Colour bgColor;
	bool dimAlphaOnDisabled;
	bool highlightOnMouseOver;

	bool removeOnDelKey;

	virtual void mouseExit(const MouseEvent &e) override;
	virtual bool keyPressed(const KeyPress &e) override;

	void setHighlightOnMouseOver(bool highlight);

	void paint(Graphics &g) override;


	virtual void newMessage(const ContainerAsyncEvent &e) override;

	//void controllableFeedbackUpdate(ControllableContainer *, Controllable *) override;
	virtual void containerChildAddressChangedAsync(ControllableContainer *) {}
	virtual void controllableFeedbackUpdateInternal(Controllable *) {} //override this in child classes
	

};





template<class T>
BaseItemMinimalUI<T>::BaseItemMinimalUI(T * _item) :
	InspectableContentComponent(_item),
	item(_item),
	bgColor(BG_COLOR.brighter(.1f)),
	dimAlphaOnDisabled(true),
	highlightOnMouseOver(false),
	removeOnDelKey(true)
{

    baseItem = static_cast<BaseItem *>(item);
    
	//setWantsKeyboardFocus(true);

	addMouseListener(this, true);
	baseItem->addAsyncContainerListener(this);
	
	if (baseItem->canBeDisabled && dimAlphaOnDisabled) setAlpha(baseItem->enabled->boolValue() ? 1 : .5f);

	//setSize(100, 20);
}

template<class T>
BaseItemMinimalUI<T>::~BaseItemMinimalUI()
{
	baseItem->removeAsyncContainerListener(this);
}

template<class T>
void BaseItemMinimalUI<T>::mouseExit(const MouseEvent &e)
{
	InspectableContentComponent::mouseExit(e);
	repaint();
}

template<class T>
bool BaseItemMinimalUI<T>::keyPressed(const KeyPress & e)
{
	if (e.getKeyCode() == e.deleteKey || e.getKeyCode() == e.backspaceKey)
	{
		if (removeOnDelKey && inspectable->isSelected && inspectable->selectionManager == InspectableSelectionManager::activeSelectionManager)
		{
			if (this->baseItem->askConfirmationBeforeRemove && GlobalSettings::getInstance()->askBeforeRemovingItems->boolValue())
			{
				int result = AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon, "Delete " + this->baseItem->niceName, "Are you sure you want to delete this ?", "Delete", "Cancel");
				if (result != 0)this->baseItem->remove();
			} else
			{
				this->baseItem->remove();
			}

			return true;
		}
	}/* else if (e.getModifiers().isCommandDown())
	{
		if (e.getKeyCode() == KeyPress::createFromDescription("d").getKeyCode())
		{	
			item->duplicate();
			return true;
		} else if(e.getKeyCode()  == KeyPress::createFromDescription("c").getKeyCode())
		{
			item->copy();
			return true;
		} else if (e.getKeyCode() == KeyPress::createFromDescription("x").getKeyCode())
		{
			item->copy();
			this->baseItem->remove();
			return true;

		} else if (e.getKeyCode() == KeyPress::createFromDescription("v").getKeyCode())
		{
			item->paste();
			return true;
		}
	}
	*/

	return false;
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

	Colour c = isItemEnabled ? bgColor : bgColor.darker(.3f);
	if (highlightOnMouseOver && isMouseOverOrDragging(true)) c = c.brighter(.03f);
	g.setColour(c);
	g.fillRoundedRectangle(r, 4);
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
		}

		controllableFeedbackUpdateInternal(e.targetControllable);
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


#endif  // BASEITEMMINIMALUI_H_INCLUDED
