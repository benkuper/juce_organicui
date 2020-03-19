#pragma once



class DashboardCCItemUI :
	public DashboardInspectableItemUI
{
public:
	DashboardCCItemUI(DashboardCCItem* ccItem);
	~DashboardCCItemUI();

	DashboardCCItem* ccItem;
	std::unique_ptr<Component> itemUI;

	virtual void paint(Graphics& g) override;
	virtual void resizedDashboardItemInternal() override;

	virtual void updateEditModeInternal(bool editMode) override;

	virtual void inspectableChanged() override;
};