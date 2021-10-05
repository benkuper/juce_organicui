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
	virtual void controllableStateUpdateInternal(Controllable* c) override;
};

/*
class ParameterColorStyleUI :
	public ParameterUI
{
public:
	ParameterColorStyleUI(Parameter* p, bool isCircle, DashboardParameterItem * item);
	~ParameterColorStyleUI();

	bool isCircle;
	DashboardParameterItem * item;

	void paint(Graphics& g) override;

	void valueChanged(const var &v) override;
};
*/