#include "DashboardControllableItemUI.h"

DashboardControllableItemUI::DashboardControllableItemUI(DashboardControllableItem* controllableItem) :
	DashboardInspectableItemUI(controllableItem),
	controllableItem(controllableItem),
	itemUI(nullptr)
{
	setSize(item->viewUISize->x, item->viewUISize->y);
}

DashboardControllableItemUI::~DashboardControllableItemUI()
{
}

void DashboardControllableItemUI::paint(Graphics& g)
{
	DashboardInspectableItemUI::paint(g);
}

void DashboardControllableItemUI::resizedDashboardItemInternal()
{
	if(itemUI != nullptr) itemUI->setBounds(getLocalBounds());
}

ControllableUI* DashboardControllableItemUI::createControllableUI()
{
	return controllableItem->controllable->createDefaultUI();
}

void DashboardControllableItemUI::rebuildUI()
{
	if (itemUI != nullptr)
	{
		removeChildComponent(itemUI.get());
	}

	if (!inspectable.wasObjectDeleted() && controllableItem->controllable != nullptr)
	{
		itemUI.reset(createControllableUI());
		addAndMakeVisible(itemUI.get());
		updateUIParameters();
		if (getWidth() == 0 || getHeight() == 0) setSize(itemUI->getWidth(), itemUI->getHeight());
	}

	updateUIParameters();
	updateEditMode();
}

void DashboardControllableItemUI::updateUIParameters()
{
	if (inspectable.wasObjectDeleted() || controllableItem->inspectable == nullptr || controllableItem->inspectable.wasObjectDeleted()) return;

	itemUI->showLabel = controllableItem->showLabel->boolValue();
	itemUI->useCustomTextColor = controllableItem->textColor->enabled; 
	itemUI->customTextColor = controllableItem->textColor->getColor();

	itemUI->customLabel = controllableItem->customLabel->enabled ? controllableItem->customLabel->stringValue() : "";
	itemUI->customDescription = controllableItem->customDescription->enabled ? controllableItem->customDescription->stringValue() : "";

	itemUI->setOpaqueBackground(controllableItem->opaqueBackground->boolValue());

	itemUI->updateTooltip();
	itemUI->repaint();
}


void DashboardControllableItemUI::updateEditModeInternal(bool editMode)
{
	if(itemUI != nullptr) itemUI->setInterceptsMouseClicks(!editMode, !editMode);
}

void DashboardControllableItemUI::inspectableChanged()
{
	DashboardInspectableItemUI::inspectableChanged();
	rebuildUI();
}

void DashboardControllableItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	DashboardInspectableItemUI::controllableFeedbackUpdateInternal(c);

	if (c == controllableItem->showLabel 
		|| c == controllableItem->textColor 
		|| c == controllableItem->customLabel 
		|| c == controllableItem->customDescription
		|| c == controllableItem->opaqueBackground)
	{
		updateUIParameters();
	}
}
