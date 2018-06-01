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

	if (item->canBeReorderedInEditor && !isRoot)
	{
		upBT = AssetManager::getInstance()->getUpBT();
		downBT = AssetManager::getInstance()->getDownBT();
		addAndMakeVisible(upBT);
		addAndMakeVisible(downBT);
		upBT->addListener(this);
		downBT->addListener(this);
	}
}

BaseItemEditor::~BaseItemEditor()
{
	if (!inspectable.wasObjectDeleted()) item->removeAsyncContainerListener(this);
}

void BaseItemEditor::setIsFirst(bool value)
{
	isFirst = value;
	if (upBT != nullptr) upBT->setVisible(!isFirst);
}

void BaseItemEditor::setIsLast(bool value)
{
	isLast = value;
	if (downBT != nullptr) downBT->setVisible(!isLast);
}

void BaseItemEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (item->userCanRemove && removeBT != nullptr)
	{
		removeBT->setBounds(r.removeFromRight(r.getHeight()).reduced(3));
		r.removeFromRight(2);
	}

	if (upBT != nullptr)
	{
		upBT->setBounds(r.removeFromRight(r.getHeight()).reduced(4));
	}

	if (downBT != nullptr)
	{
		downBT->setBounds(r.removeFromRight(r.getHeight()).reduced(4));
	}

	resizedInternalHeaderItemInternal(r);

	EnablingControllableContainerEditor::resizedInternalHeader(r); 
}

void BaseItemEditor::buttonClicked(Button * b)
{
	EnablingControllableContainerEditor::buttonClicked(b);

	if (b == removeBT)
	{ 
		if (this->item->askConfirmationBeforeRemove && GlobalSettings::getInstance()->askBeforeRemovingItems->boolValue())
		{
			int result = AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon, "Delete " + this->item->niceName, "Are you sure you want to delete this ?", "Delete", "Cancel");
			if (result != 0)this->item->remove();
		} else this->item->remove();
		return;

	} else if (b == upBT)
	{
		item->moveBefore();
	} else if (b == downBT)
	{
		item->moveAfter();
	}
}

