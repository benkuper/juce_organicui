
DashboardControllableItemUI::DashboardControllableItemUI(DashboardControllableItem* controllableItem) :
	DashboardInspectableItemUI(controllableItem),
	controllableItem(controllableItem),
	itemUI(nullptr)
{
	inspectableChanged(); //force check inspectable
	if(itemUI != nullptr) setSize(item->viewUISize->x, item->viewUISize->y);
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

void DashboardControllableItemUI::updateEditModeInternal(bool editMode)
{
	if(itemUI != nullptr) itemUI->setInterceptsMouseClicks(!editMode, !editMode);
}

void DashboardControllableItemUI::inspectableChanged()
{
	DashboardInspectableItemUI::inspectableChanged();

	if (itemUI != nullptr)
	{
		removeChildComponent(itemUI.get());
	}

	if (!inspectable.wasObjectDeleted() && controllableItem->controllable != nullptr)
	{
		itemUI.reset(controllableItem->controllable->createDefaultUI());
		addAndMakeVisible(itemUI.get());
		itemUI->showLabel = true;
		if (getWidth() == 0 || getHeight() == 0) setSize(itemUI->getWidth(), itemUI->getHeight());
	}

	updateEditMode();
}
