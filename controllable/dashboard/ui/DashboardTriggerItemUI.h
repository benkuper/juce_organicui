#pragma once


class DashboardTriggerItemUI :
	public DashboardControllableItemUI
{
public:
	DashboardTriggerItemUI(DashboardTriggerItem* triggerItem);
	~DashboardTriggerItemUI();

	DashboardTriggerItem* triggerItem;

	virtual ControllableUI* createControllableUI() override;
	virtual void updateUIParameters() override;

	virtual void controllableFeedbackUpdateInternal(Controllable* c) override;

};