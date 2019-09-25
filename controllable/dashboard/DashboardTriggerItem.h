#pragma once


class DashboardTriggerItem :
	public DashboardControllableItem
{
public:
	DashboardTriggerItem(Trigger* item = nullptr);
	~DashboardTriggerItem();

	Trigger* trigger;

	FileParameter * customImagePath;

	virtual DashboardItemUI* createUI() override;

	virtual void setInspectableInternal(Inspectable* i) override;


	static DashboardTriggerItem* create(var) { return new DashboardTriggerItem(); }
	virtual String getTypeString() const override { return "DashboardTriggerItem"; }

};