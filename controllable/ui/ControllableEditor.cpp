#include "ControllableEditor.h"
/*
  ==============================================================================

	ControllableEditor.cpp
	Created: 7 Oct 2016 2:04:37pm
	Author:  bkupe

  ==============================================================================
*/


ControllableEditor::ControllableEditor(Controllable * _controllable, bool isRoot, int initHeight) :
	InspectableEditor(_controllable, isRoot),
	controllable(_controllable),
	label("Label"),
    subContentHeight(0),
    showLabel(true)
{
	ui = controllable->createDefaultUI();
	ui->showLabel = false;
	ui->setOpaqueBackground(true);
	addAndMakeVisible(ui);

	addAndMakeVisible(&label);
	label.setJustificationType(Justification::left);
	label.setFont(label.getFont().withHeight(12));
	label.setText(controllable->niceName, dontSendNotification);
	label.setTooltip(controllable->niceName+"\n"+controllable->description);

	if (controllable->isRemovableByUser)
	{
		removeBT = AssetManager::getInstance()->getRemoveBT();
		removeBT->addListener(this);
		addAndMakeVisible(removeBT);
	}

	if (controllable->isCustomizableByUser)
	{
		editBT = AssetManager::getInstance()->getConfigBT();
		editBT->addListener(this);
		addAndMakeVisible(editBT);
	}

	baseHeight = initHeight;
	setSize(100, baseHeight);

	controllable->addAsyncControllableListener(this);
}

ControllableEditor::~ControllableEditor()
{
	if (controllable.wasObjectDeleted()) return;
	controllable->removeAsyncControllableListener(this);
}

void ControllableEditor::setShowLabel(bool value)
{
	if (showLabel == value) return;
	showLabel = value;
	if (showLabel)
	{
		addAndMakeVisible(&label);
	} else
	{
		removeChildComponent(&label);
	}

}

void ControllableEditor::resized()
{
 juce::Rectangle<int> r = getLocalBounds();
	r.removeFromBottom(subContentHeight);// .withHeight(16);

	int controlSpace = jmax<int>(showLabel?getWidth()*.6f:getWidth(), 100);


	if (controllable->isRemovableByUser && removeBT != nullptr)
	{
		removeBT->setBounds(r.removeFromRight(r.getHeight()));
		r.removeFromRight(2);
	}

	if (controllable->isCustomizableByUser && editBT != nullptr)
	{
		editBT->setBounds(r.removeFromRight(r.getHeight()));
		r.removeFromRight(2);
	}


	ui->setBounds(r.removeFromRight(controlSpace));
	
	if (showLabel)
	{
		r.removeFromRight(2); 
		label.setBounds(r);
	}

}
void ControllableEditor::mouseDown(const MouseEvent & e)
{
	if (e.eventComponent == &label)
	{
		if (ui->showMenuOnRightClick) ui->showContextMenu();
	}
}

void ControllableEditor::newMessage(const Controllable::ControllableEvent & e)
{
	if (e.type == Controllable::ControllableEvent::NAME_CHANGED)
	{
		label.setText(controllable->niceName, dontSendNotification);
	}
}

void ControllableEditor::buttonClicked(Button * b)
{
	if (b == removeBT)
	{
		controllable->remove(true);
	} else if (b == editBT)
	{
		ui->showEditWindow();
	}
}
