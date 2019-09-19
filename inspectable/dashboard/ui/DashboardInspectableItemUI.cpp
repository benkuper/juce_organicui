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
	if (inspectableItem->inspectable == nullptr && inspectableItem->inspectableGhostAddress.isNotEmpty())
	{
		g.setColour(TEXT_COLOR);
		g.drawFittedText("Missing : " + inspectableItem->inspectableGhostAddress, getLocalBounds(), Justification::centred, 2);
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