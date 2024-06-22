DashboardInspectableItemUI::DashboardInspectableItemUI(DashboardInspectableItem* inspectableItem) :
	DashboardItemUI(inspectableItem),
	inspectableItem(inspectableItem)
{
	inspectableItem->addAsyncInspectableItemListener(this);
	
}

DashboardInspectableItemUI::~DashboardInspectableItemUI()
{
	if (inspectableItem != nullptr) inspectableItem->removeAsyncInspectableItemListener(this);
}

void DashboardInspectableItemUI::paint(Graphics& g)
{
	DashboardItemUI::paint(g);

	if (inspectableItem != nullptr && (inspectableItem->inspectable.wasObjectDeleted() || inspectableItem->inspectable == nullptr))
	{
		g.fillAll(Colours::white.withAlpha(.1f));
		g.setColour(RED_COLOR);
		g.setFont(FontOptions(14));
		g.drawFittedText("#Missing : " + inspectableItem->inspectableGhostAddress, getLocalBounds(), Justification::centred, 3);
	}
}

void DashboardInspectableItemUI::newMessage(const DashboardInspectableItem::InspectableItemEvent& e)
{
	switch (e.type)
	{
	case DashboardInspectableItem::InspectableItemEvent::INSPECTABLE_CHANGED:
		inspectableChanged();

	}
}