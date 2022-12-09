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
	
	void paint(Graphics& g) override;
	void resizedInternalHeaderItemInternal(Rectangle<int>& r) override;
	void resetAndBuild() override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent &e) override;

	virtual void setDragDetails(var& details) override;

	void childBoundsChanged(Component* c) override;
};
