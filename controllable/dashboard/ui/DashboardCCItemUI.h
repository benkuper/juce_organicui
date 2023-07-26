#pragma once



class DashboardCCItemUI :
	public DashboardInspectableItemUI
{
public:
	DashboardCCItemUI(DashboardCCItem* ccItem);
	~DashboardCCItemUI();

	DashboardCCItem* ccItem;
	juce::Viewport vp;
	juce::Component * itemUI;

	virtual void paint(juce::Graphics& g) override;
	virtual void resizedDashboardItemInternal() override;

	virtual void updateEditModeInternal(bool editMode) override;

	virtual void inspectableChanged() override;
};