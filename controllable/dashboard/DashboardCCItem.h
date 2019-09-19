#pragma once


class DashboardCCItem :
	public DashboardInspectableItem
{
public:
	DashboardCCItem(ControllableContainer* container = nullptr);
	~DashboardCCItem();

	WeakReference<ControllableContainer> container;

	virtual var getJSONData();
	virtual void loadJSONDataItemInternal(var data) override;

	virtual void setInspectableInternal(Inspectable* i) override;

	virtual DashboardItemUI* createUI() override;

	static DashboardCCItem* create(var) { return new DashboardCCItem(); }
	virtual String getTypeString() const override { return "DashboardCCItem"; }
};