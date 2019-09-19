#pragma once


class DashboardControllableItemUI :
	public DashboardInspectableItemUI
{
public:
	DashboardControllableItemUI(DashboardControllableItem * controllableItem);
	~DashboardControllableItemUI();

	DashboardControllableItem* controllableItem;
	std::unique_ptr<ControllableUI> itemUI;

	virtual void paint(Graphics& g) override;
	virtual void resizedDashboardItemInternal() override;

	virtual void updateEditModeInternal(bool editMode) override;

	virtual void inspectableChanged() override;
};