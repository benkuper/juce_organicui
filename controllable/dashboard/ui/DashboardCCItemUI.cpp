
DashboardCCItemUI::DashboardCCItemUI(DashboardCCItem* ccItem) :
	DashboardInspectableItemUI(ccItem),
	ccItem(ccItem),
	itemUI(nullptr)
{
	vp.setScrollBarsShown(true, false);
	vp.setScrollOnDragMode(Viewport::ScrollOnDragMode::never);
	vp.setScrollBarThickness(10);
	addAndMakeVisible(vp);

	resized();

	inspectableChanged(); //force check inspectable
	setSize(item->viewUISize->x, item->viewUISize->y);
}

DashboardCCItemUI::~DashboardCCItemUI()
{
	itemUI = nullptr;
	vp.setViewedComponent(nullptr);
}

void DashboardCCItemUI::paint(Graphics& g)
{
	DashboardInspectableItemUI::paint(g);
}

void DashboardCCItemUI::resizedDashboardItemInternal()
{
	vp.setBounds(getLocalBounds());
	if (itemUI != nullptr) itemUI->setBounds(juce::Rectangle<int>(0,0,getWidth()-10,itemUI->getHeight()));
}

void DashboardCCItemUI::updateEditModeInternal(bool editMode)
{
	if (itemUI != nullptr)
	{
		setTooltip((editMode && !ccItem->container.wasObjectDeleted()) ? "Target : " + ccItem->container->getControlAddress() : "");
		itemUI->setInterceptsMouseClicks(!editMode, !editMode);
	}
	else setTooltip("### " + ccItem->inspectableGhostAddress);
}

void DashboardCCItemUI::inspectableChanged()
{
	DashboardInspectableItemUI::inspectableChanged();

	if (itemUI != nullptr)
	{
		vp.setViewedComponent(nullptr);
		itemUI = nullptr;
	}

	if (!inspectable.wasObjectDeleted() && ccItem->container != nullptr)
	{
		InspectableEditor * e = ccItem->container->getEditor(true);
		GenericControllableContainerEditor* gce = dynamic_cast<GenericControllableContainerEditor*>(e);
		if (gce != nullptr) gce->setDragAndDropEnabled(false);

		itemUI = e;
		vp.setViewedComponent(itemUI);
		
		if (getWidth() == 0 || getHeight() == 0) setSize(itemUI->getWidth(), itemUI->getHeight());
	}

	updateEditMode();
}
