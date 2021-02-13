
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

	var getServerData() override;

	var getJSONData() override;
	void loadJSONDataItemInternal(var data) override;

	bool paste() override;
	
	virtual DashboardItemUI* createUI() override;

	String getTypeString() const override { return "DashboardGroupItem"; }
	static DashboardGroupItem* create(var) { return new DashboardGroupItem(); }
};