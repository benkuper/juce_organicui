/*
  ==============================================================================

    DashboardTargetItemUI.h
    Created: 19 Apr 2017 11:01:13pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardTargetItemUI :
	public BaseItemUI<DashboardItem>
{
public:
	DashboardTargetItemUI(DashboardTargetItem * item);
	~DashboardTargetItemUI();

	DashboardTargetItem * targetItem;

	std::unique_ptr<Component> targetUI;
	virtual void updateTargetUI();

	virtual void resizedInternalContent(juce::Rectangle<int> &r) override;

	void controllableFeedbackUpdateInternal(Controllable * c) override;

};
