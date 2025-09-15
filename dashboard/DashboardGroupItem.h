
#pragma once

class DashboardGroupItem :
	public DashboardItem
{
public:
	DashboardGroupItem();
	virtual ~DashboardGroupItem();


	FloatParameter* borderWidth;
	ColorParameter* borderColor;
	ColorParameter* backgroundColor;

	DashboardItemManager itemManager;

	juce::var getServerData() override;

	juce::var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataItemInternal(juce::var data) override;

	bool paste() override;
	

	juce::String getTypeString() const override { return "DashboardGroupItem"; }
	static DashboardGroupItem* create(juce::var) { return new DashboardGroupItem(); }

	DECLARE_UI_FUNC;
};