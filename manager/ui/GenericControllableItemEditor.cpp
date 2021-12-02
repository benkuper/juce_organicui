/*
  ==============================================================================

	GenericControllableItemUI.cpp
	Created: 13 May 2017 3:31:56pm
	Author:  Ben

  ==============================================================================
*/

GenericControllableItemEditor::GenericControllableItemEditor(GenericControllableItem * gci, bool isRoot) :
	BaseItemEditor(gci, isRoot)
{
	for (auto &ce : childEditors)
	{
		ControllableEditor * c = dynamic_cast<ControllableEditor *>(ce);
		if (c != nullptr) c->setShowLabel(false);
	}
}

GenericControllableItemEditor::~GenericControllableItemEditor()
{
}

void GenericControllableItemEditor::resetAndBuild()
{
	BaseItemEditor::resetAndBuild();

	for (auto &ce : childEditors)
	{
		ControllableEditor * c = dynamic_cast<ControllableEditor *>(ce);
		if (c != nullptr) c->setShowLabel(false);
	}
}
