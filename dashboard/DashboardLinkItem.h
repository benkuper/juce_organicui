
#pragma once

class DashboardLinkItem :
	public DashboardTriggerItem
{
public:
	DashboardLinkItem();
	virtual ~DashboardLinkItem();

	TargetParameter * dashboardTarget;
	BoolParameter* setInClients;
	Trigger* launchTrigger;

	void onContainerTriggerTriggered(Trigger * t) override;

	var getServerData() override;

	virtual DashboardItemUI* createUI() override;

	String getTypeString() const override { return "DashboardLinkItem"; }
	static DashboardLinkItem* create(var) { return new DashboardLinkItem(); }
};