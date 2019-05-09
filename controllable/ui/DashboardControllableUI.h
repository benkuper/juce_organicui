/*
  ==============================================================================

    DashboardItemUI.h
    Created: 19 Apr 2017 11:01:13pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardControllableUI :
	public DashboardItemUI
{
public:
	DashboardControllableUI(DashboardItem * item);
	~DashboardControllableUI();

	ScopedPointer<ControllableUI> controllableUI;
	virtual void updateTargetUI() override;

	void resizedInternalContent(juce::Rectangle<int> &r) override;
};
