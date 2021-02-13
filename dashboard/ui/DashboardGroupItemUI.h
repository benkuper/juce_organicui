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

	void paint(Graphics &g) override;

	void resizedDashboardItemInternal() override;
	void updateEditModeInternal(bool editMode) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent &e) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;

	bool canStartDrag(const MouseEvent& e) override;
};