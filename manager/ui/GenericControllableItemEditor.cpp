#include "GenericControllableItemEditor.h"
/*
  ==============================================================================

	GenericControllableItemUI.cpp
	Created: 13 May 2017 3:31:56pm
	Author:  Ben

  ==============================================================================
*/

GenericControllableItemEditor::GenericControllableItemEditor(BaseItem * bi, bool isRoot) :
	BaseItemEditor(bi, isRoot),
	gci((GenericControllableItem *)bi)
{
	cui = static_cast<ControllableEditor *>(gci->controllable->getEditor(false));
	cui->setShowLabel(false);
	addAndMakeVisible(cui);
}

GenericControllableItemEditor::~GenericControllableItemEditor()
{
}

void GenericControllableItemEditor::resizedInternalHeader(Rectangle<int>& r)
{
	cui->setBounds(r.removeFromRight(r.getWidth()-100)); 
}
