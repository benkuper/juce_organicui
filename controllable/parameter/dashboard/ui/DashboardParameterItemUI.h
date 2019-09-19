#pragma once


class DashboardParameterItemUI :
	public DashboardControllableItemUI
{
public:
	DashboardParameterItemUI(DashboardParameterItem* item);
	~DashboardParameterItemUI();

	DashboardParameterItem * parameterItem;
};