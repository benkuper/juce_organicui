
#pragma once

class DashboardLinkItem :
	public DashboardTriggerItem
{
public:
	DashboardLinkItem();
	virtual ~DashboardLinkItem();

	TargetParameter * dashboardTarget;
	BoolParameter* setInNative;
	BoolParameter* setInClients;
	Trigger* launchTrigger;

	void onContainerTriggerTriggered(Trigger * t) override;

	juce::var getServerData() override;

	virtual DashboardItemUI* createUI() override;

	juce::String getTypeString() const override { return "DashboardLinkItem"; }
	static DashboardLinkItem* create(juce::var) { return new DashboardLinkItem(); }
};