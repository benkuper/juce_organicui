
#pragma once

class DashboardGroupItem :
	public DashboardItem
{
public:
	DashboardGroupItem();
	virtual ~DashboardGroupItem();

	DashboardItemManager itemManager;

	virtual DashboardItemUI* createUI() override;
};