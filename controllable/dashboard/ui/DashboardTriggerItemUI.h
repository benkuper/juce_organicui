#pragma once


class DashboardTriggerItemUI :
	public DashboardControllableItemUI
{
public:
	DashboardTriggerItemUI(DashboardTriggerItem* triggerItem);
	~DashboardTriggerItemUI();

	DashboardTriggerItem* triggerItem;
};