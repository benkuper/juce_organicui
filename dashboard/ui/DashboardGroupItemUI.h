/*
  ==============================================================================

    DashboardUI.h
    Created: 19 Apr 2017 11:06:43pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardGroupItemUI :
	public DashboardItemUI
{
public:
	DashboardGroupItemUI(DashboardGroupItem * group);
	~DashboardGroupItemUI();

	DashboardGroupItem* group;
	std::unique_ptr<DashboardItemManagerUI> managerUI;

	void paint(juce::Graphics &g) override;

	void resizedDashboardItemInternal() override;
	void updateEditModeInternal(bool editMode) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent &e) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;

	bool canStartDrag(const juce::MouseEvent& e) override;
};