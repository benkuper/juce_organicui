/*
  ==============================================================================

	GenericControllableItemUI.cpp
	Created: 13 May 2017 3:31:56pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

GenericControllableItemEditor::GenericControllableItemEditor(GenericControllableItem* gci, bool isRoot) :
	BaseItemEditor(gci, isRoot)
{
	contourColor = Colours::transparentBlack;

	containerLabel.setVisible(false);

	if (gci->controllable != nullptr)
	{
		controllableEditor.reset((ControllableEditor*)gci->controllable->getEditor(false));
		//controllableEditor->setShowLabel(false);
		addAndMakeVisible(controllableEditor.get());

		headerHeight = controllableEditor->getHeight() + 2;
	}

	//containerLabel.setInterceptsMouseClicks(false, false);
	//containerLabel.addMouseListener(this, true);

	//for (auto &ce : childEditors)
	//{
	//	ControllableEditor * c = dynamic_cast<ControllableEditor *>(ce);
	//	if (c != nullptr) c->setShowLabel(false);
	//}
}

GenericControllableItemEditor::~GenericControllableItemEditor()
{
	if (!controllableEditor->inspectable.wasObjectDeleted())
	{
		//if (Parameter* p = dynamic_cast<Parameter*>(controllableEditor->controllable.get())) p->AsyncParameterListener(this);

	}
}

void GenericControllableItemEditor::paint(juce::Graphics& g)
{
	BaseItemEditor::paint(g);
	//nothing
}

void GenericControllableItemEditor::resizedInternalHeaderItemInternal(juce::Rectangle<int>& r)
{
	//int labelWidth = containerLabel.getWidth();
	//r.removeFromLeft(jmax(160 - labelWidth, 0));
	//r.translate(0, 1);
	controllableEditor->setBounds(r);// .withHeight(controllableEditor->getHeight()).reduced(1, 0));
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
	BaseItemEditor::mouseDrag(e);

	//if (dragAndDropEnabled && e.eventComponent != controllableEditor.get())
	//{
	//	var desc = var(new DynamicObject());
	//	desc.getDynamicObject()->setProperty("type", controllableEditor->controllable->getTypeString());
	//	desc.getDynamicObject()->setProperty("dataType", "Controllable");
	//	//Image dragImage = this->createComponentSnapshot(this->getLocalBounds()).convertedToFormat(Image::ARGB);
	//	//dragImage.multiplyAllAlphas(.5f);
	//	Point<int> offset = -getMouseXYRelative();
	//	if (e.getDistanceFromDragStart() > 30) startDragging(desc, controllableEditor.get(), ScaledImage(), true, &offset);
	//}
	//else
	//{
	//	BaseItemEditor::mouseDrag(e);
	//}
}

void GenericControllableItemEditor::setDragDetails(var& details)
{
	details.getDynamicObject()->setProperty("type", controllableEditor->controllable->getTypeString());
	details.getDynamicObject()->setProperty("dataType", "Controllable");// item->itemDataType);
}

void GenericControllableItemEditor::childBoundsChanged(Component* c)
{
	if (isRebuilding) return;
	if (getWidth() == 0 || getHeight() == 0) return;

	if (c == controllableEditor.get()) headerHeight = controllableEditor->getHeight() + 2;
	BaseItemEditor::childBoundsChanged(c);
}
