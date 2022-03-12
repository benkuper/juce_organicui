#include "GenericControllableItemEditor.h"
/*
  ==============================================================================

	GenericControllableItemUI.cpp
	Created: 13 May 2017 3:31:56pm
	Author:  Ben

  ==============================================================================
*/

GenericControllableItemEditor::GenericControllableItemEditor(GenericControllableItem* gci, bool isRoot) :
	BaseItemEditor(gci, isRoot)
{
	//containerLabel.setVisible(false);

	if (gci->controllable != nullptr)
	{
		controllableEditor.reset((ControllableEditor*)gci->controllable->getEditor(false));
		controllableEditor->setShowLabel(false);
		addAndMakeVisible(controllableEditor.get());
	}

	//containerLabel.setInterceptsMouseClicks(false, false);
	containerLabel.addMouseListener(this, true);
	
	//for (auto &ce : childEditors)
	//{
	//	ControllableEditor * c = dynamic_cast<ControllableEditor *>(ce);
	//	if (c != nullptr) c->setShowLabel(false);
	//}
}

GenericControllableItemEditor::~GenericControllableItemEditor()
{
}

void GenericControllableItemEditor::paint(Graphics& g)
{
	//nothing
}

void GenericControllableItemEditor::resizedInternalHeaderItemInternal(Rectangle<int>& r)
{
	controllableEditor->setBounds(r.reduced(1));
}

void GenericControllableItemEditor::resetAndBuild()
{
	
	//BaseItemEditor::resetAndBuild();

	//for (auto &ce : childEditors)
	//{
	//	ControllableEditor * c = dynamic_cast<ControllableEditor *>(ce);
	//	if (c != nullptr) c->setShowLabel(false);
	//}
}

void GenericControllableItemEditor::mouseDown(const MouseEvent& e)
{
	BaseItemEditor::mouseDown(e);
}

void GenericControllableItemEditor::mouseDrag(const MouseEvent& e)
{
	if (dragAndDropEnabled && e.eventComponent != controllableEditor.get())
	{
		var desc = var(new DynamicObject());
		desc.getDynamicObject()->setProperty("type", controllableEditor->controllable->getTypeString());
		desc.getDynamicObject()->setProperty("dataType", "Controllable");
		//Image dragImage = this->createComponentSnapshot(this->getLocalBounds()).convertedToFormat(Image::ARGB);
		//dragImage.multiplyAllAlphas(.5f);
		Point<int> offset = -getMouseXYRelative();
		if (e.getDistanceFromDragStart() > 30) startDragging(desc, controllableEditor.get(), ScaledImage(), true, &offset);
	}
	else
	{
		BaseItemEditor::mouseDrag(e);
	}
}
