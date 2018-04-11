/*
  ==============================================================================

    GenericControllableItemUI.h
    Created: 13 May 2017 3:31:56pm
    Author:  Ben

  ==============================================================================
*/

#ifndef GENERICCONTROLLABLEITEMUI_H_INCLUDED
#define GENERICCONTROLLABLEITEMUI_H_INCLUDED

class GenericControllableItemUI :
	public BaseItemUI<GenericControllableItem>
{
public:
	GenericControllableItemUI(GenericControllableItem * bi);
	~GenericControllableItemUI();

	ScopedPointer<ControllableEditor> cui;

	void resizedInternalHeader(juce::Rectangle<int> &r) override;
	void resizedInternalContent(juce::Rectangle<int> &r) override;
	void childBoundsChanged(Component * child) override;
};

#endif  // GENERICCONTROLLABLEITEMUI_H_INCLUDED
