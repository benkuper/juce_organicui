/*
  ==============================================================================

    GenericControllableItemUI.h
    Created: 13 May 2017 3:31:56pm
    Author:  Ben

  ==============================================================================
*/

#ifndef GENERICCONTROLLABLEITEMUI_H_INCLUDED
#define GENERICCONTROLLABLEITEMUI_H_INCLUDED

class GenericControllableItemEditor :
	public BaseItemEditor
{
public:
	GenericControllableItemEditor(BaseItem * bi, bool isRoot);
	~GenericControllableItemEditor();

	GenericControllableItem * gci;
	ScopedPointer<ControllableEditor> cui;

	void resizedInternalHeader(Rectangle<int> &r) override;
	void resizedInternalContent(Rectangle<int> &r) override;
	void childBoundsChanged(Component * child) override;
};

#endif  // GENERICCONTROLLABLEITEMUI_H_INCLUDED
