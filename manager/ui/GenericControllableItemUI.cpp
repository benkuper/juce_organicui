/*
  ==============================================================================

	GenericControllableItemUI.cpp
	Created: 13 May 2017 3:31:56pm
	Author:  Ben

  ==============================================================================
*/

GenericControllableItemUI::GenericControllableItemUI(GenericControllableItem * bi) :
	BaseItemUI<GenericControllableItem>(bi)
{
	cui = static_cast<ControllableEditor *>(item->controllable->getEditor(false));
	cui->setShowLabel(false);
	addAndMakeVisible(cui);

	setContentSize(getWidth(), cui->getHeight());
}

GenericControllableItemUI::~GenericControllableItemUI()
{
}

void GenericControllableItemUI::resizedInternalHeader(Rectangle<int>& r)
{
	
}

void GenericControllableItemUI::resizedInternalContent(Rectangle<int>& r)
{
	
	r.setHeight(cui->getHeight());
	cui->setBounds(r);
}

void GenericControllableItemUI::childBoundsChanged(Component * child)
{
	if (child == cui)
	{
		int contentHeight = getHeight() - getHeightWithoutContent();
		if (contentHeight == cui->getHeight()) return;

		setContentSize(getWidth(), cui->getHeight());
	}
}
