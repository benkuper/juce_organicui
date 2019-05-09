#include "DashboardControllableUI.h"
/*
  ==============================================================================

    DashboardControllableUI.cpp
    Created: 19 Apr 2019 11:01:13pm
    Author:  Ben

  ==============================================================================
*/


DashboardControllableUI::DashboardControllableUI(DashboardItem * item) :
	DashboardItemUI(item),
	controllableUI(nullptr)
{
	updateTargetUI();
}

DashboardControllableUI::~DashboardControllableUI()
{
}

void DashboardControllableUI::updateTargetUI()
{
	if (controllableUI != nullptr)
	{
		removeChildComponent(controllableUI);
		controllableUI = nullptr;
	}

	if (item->target != nullptr && !item->target.wasObjectDeleted())
	{
		controllableUI = ((Controllable *)item->target.get())->createDefaultUI();
		if (controllableUI != nullptr)
		{
			controllableUI->showLabel = false;
			addAndMakeVisible(controllableUI);
			if (getWidth() == 0) setSize(controllableUI->getWidth()+10, controllableUI->getHeight()+10);
		}
	}
	resized();
}

void DashboardControllableUI::resizedInternalContent(juce::Rectangle<int>& r)
{
	if (controllableUI != nullptr) controllableUI->setBounds(r);
}