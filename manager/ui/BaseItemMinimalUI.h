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
	Colour selectedColor;

	bool dimAlphaOnDisabled;
	bool highlightOnMouseOver;
	bool fillColorOnSelected;

	bool removeOnDelKey;

	virtual void mouseExit(const MouseEvent &e) override;

	void setHighlightOnMouseOver(bool highlight);

	void paint(Graphics &g) override;


	virtual void newMessage(const ContainerAsyncEvent &e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer *) {}
	virtual void controllableFeedbackUpdateInternal(Controllable *) {} //override this in child classes

};





template<class T>
BaseItemMinimalUI<T>::BaseItemMinimalUI(T * _item) :
	InspectableContentComponent(_item),
	item(_item),
	bgColor(BG_COLOR.brighter(.1f)),
	selectedColor(HIGHLIGHT_COLOR),
	dimAlphaOnDisabled(true),
	highlightOnMouseOver(false),
	fillColorOnSelected(false),
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
