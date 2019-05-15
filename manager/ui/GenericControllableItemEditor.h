/*
  ==============================================================================

    GenericControllableItemUI.h
    Created: 13 May 2017 3:31:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "BaseItemEditor.h"

class GenericControllableItemEditor :
	public BaseItemEditor
{
public:
	GenericControllableItemEditor(GenericControllableItem * gci, bool isRoot);
	~GenericControllableItemEditor();

	std::unique_ptr<ControllableEditor> controllableEditor;

	void resetAndBuild() override;
};
