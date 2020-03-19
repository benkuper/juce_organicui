
DashboardCCItemUI::DashboardCCItemUI(DashboardCCItem* ccItem) :
	DashboardInspectableItemUI(ccItem),
	ccItem(ccItem),
	itemUI(nullptr)
{
	inspectableChanged(); //force check inspectable
	if (itemUI != nullptr) setSize(item->viewUISize->x, item->viewUISize->y);
}

DashboardCCItemUI::~DashboardCCItemUI()
{
}

void DashboardCCItemUI::paint(Graphics& g)
{
	DashboardInspectableItemUI::paint(g);
}

void DashboardCCItemUI::resizedDashboardItemInternal()
{
	if (itemUI != nullptr) itemUI->setBounds(getLocalBounds());
}

void DashboardCCItemUI::updateEditModeInternal(bool editMode)
{
	if (itemUI != nullptr) itemUI->setInterceptsMouseClicks(!editMode, !editMode);
}

void DashboardCCItemUI::inspectableChanged()
{
	DashboardInspectableItemUI::inspectableChanged();

	if (itemUI != nullptr)
	{
		removeChildComponent(itemUI.get());
	}

	if (!inspectable.wasObjectDeleted() && ccItem->container != nullptr)
	{
		InspectableEditor * e = ccItem->container->getEditor(true);
		GenericControllableContainerEditor* gce = dynamic_cast<GenericControllableContainerEditor*>(e);
		if (gce != nullptr) gce->setDragAndDropEnabled(false);

		itemUI.reset(e);

		addAndMakeVisible(itemUI.get());
		if (getWidth() == 0 || getHeight() == 0) setSize(itemUI->getWidth(), itemUI->getHeight());
	}

	updateEditMode();
}
