#pragma once


class DashboardTriggerItem :
	public DashboardControllableItem
{
public:
	DashboardTriggerItem(Trigger* item = nullptr);
	virtual ~DashboardTriggerItem();

	Trigger* trigger;

	ColorParameter * bgColor;
	FileParameter * customImagePath;
	BoolParameter* keepSaturation;

	virtual DashboardItemUI* createUI() override;

	virtual void setInspectableInternal(Inspectable* i) override;

	virtual void onExternalTriggerTriggered(Trigger* t) override;

	virtual juce::var getServerData() override;

	static DashboardTriggerItem* create(juce::var) { return new DashboardTriggerItem(); }
	virtual juce::String getTypeString() const override { return "DashboardTriggerItem"; }

};