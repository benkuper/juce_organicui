#pragma once


class DashboardParameterItemUI :
	public DashboardControllableItemUI
{
public:
	DashboardParameterItemUI(DashboardParameterItem* item);
	~DashboardParameterItemUI();

	DashboardParameterItem * parameterItem;

	virtual ControllableUI * createControllableUI() override;
	virtual void updateUIParameters() override;
	
	virtual void controllableFeedbackUpdateInternal(Controllable* c) override;
};