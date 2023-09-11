#pragma once


class DashboardInspectableItemUI :
	public DashboardItemUI,
	public DashboardInspectableItem::ItemAsyncListener
{
public:
	DashboardInspectableItemUI(DashboardInspectableItem * controllableItem);
	~DashboardInspectableItemUI();

	DashboardInspectableItem * inspectableItem;

	virtual void paint(juce::Graphics& g) override;

	virtual void inspectableChanged() {}

	virtual void newMessage(const DashboardInspectableItem::InspectableItemEvent& e) override;

};