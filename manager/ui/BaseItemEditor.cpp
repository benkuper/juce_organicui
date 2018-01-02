#include "BaseItemEditor.h"
/*
  ==============================================================================

	BaseItemEditor.cpp
	Created: 18 Jan 2017 2:23:31pm
	Author:  Ben

  ==============================================================================
*/


BaseItemEditor::BaseItemEditor(BaseItem * bi, bool isRoot) :
	EnablingControllableContainerEditor(bi, isRoot),
	item(bi)
{

	if (item->userCanRemove)
	{
		removeBT = AssetManager::getInstance()->getRemoveBT();
		addAndMakeVisible(removeBT);
		removeBT->addListener(this);
	}
}

BaseItemEditor::~BaseItemEditor()
{
	if (!inspectable.wasObjectDeleted()) item->removeAsyncContainerListener(this);
}

void BaseItemEditor::resizedInternalHeader(Rectangle<int>& r)
{
	if (item->userCanRemove)
	{
		removeBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
		r.removeFromRight(2);
	}

	resizedInternalHeaderItemInternal(r);

	EnablingControllableContainerEditor::resizedInternalHeader(r); 
}

void BaseItemEditor::buttonClicked(Button * b)
{
	EnablingControllableContainerEditor::buttonClicked(b);

	if (b == removeBT)
	{
		item->remove();
		return;
	}
}

