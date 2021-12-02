#pragma once


class DashboardParameterItemUI :
	public DashboardControllableItemUI
{
public:
	DashboardParameterItemUI(DashboardParameterItem* item);
	~DashboardParameterItemUI();

	DashboardParameterItem* parameterItem;

	virtual ControllableUI* createControllableUI() override;
	virtual void updateUIParametersInternal() override;

	virtual void controllableFeedbackUpdateInternal(Controllable* c) override;
	virtual void controllableStateUpdateInternal(Controllable* c) override;
};

class DashboardParameterStyleEditor :
	public ParameterEditor
{
public:
	DashboardParameterStyleEditor(Parameter* p, DashboardParameterItem * dpi, bool isRoot);
	~DashboardParameterStyleEditor();

	DashboardParameterItem* dpi;
	TextButton bt;

	void resizedInternal(juce::Rectangle<int>& r) override;
	void buttonClicked(Button* b) override;
};