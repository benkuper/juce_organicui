#pragma once


class DashboardTriggerItemUI :
	public DashboardControllableItemUI
{
public:
	DashboardTriggerItemUI(DashboardTriggerItem* triggerItem);
	~DashboardTriggerItemUI();

	DashboardTriggerItem* triggerItem;

	virtual ControllableUI* createControllableUI();

	virtual void controllableFeedbackUpdateInternal(Controllable* c) override;

};