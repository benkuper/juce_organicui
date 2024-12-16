#pragma once


class DashboardCCItem :
	public DashboardInspectableItem
{
public:
	DashboardCCItem(ControllableContainer* container = nullptr);
	~DashboardCCItem();

	juce::WeakReference<ControllableContainer> container;

	virtual juce::var getServerData() override;

	virtual juce::var getServerDataForContainer(ControllableContainer* cc);
	virtual juce::var getServerDataForControllable(Controllable* c);

	virtual juce::var getJSONData(bool includeNonOverriden = false) override;
	virtual void loadJSONDataItemInternal(juce::var data) override;

	virtual void setInspectableInternal(Inspectable* i) override;

	virtual void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) override;

	virtual DashboardItemUI* createUI() override;

	virtual void ghostInspectable() override;
	virtual void checkGhost() override;

	static DashboardCCItem* create(juce::var) { return new DashboardCCItem(); }
	virtual juce::String getTypeString() const override { return "DashboardCCItem"; }
};
