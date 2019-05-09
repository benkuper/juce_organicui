/*
  ==============================================================================

    DashboardItemUI.h
    Created: 19 Apr 2017 11:01:13pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemUI :
	public BaseItemUI<DashboardItem>,
	public DashboardItem::AsyncListener
{
public:
	DashboardItemUI(DashboardItem * item);
	~DashboardItemUI();

	virtual void updateTargetUI() {}

	void newMessage(const DashboardItem::DashboardItemEvent &e) override;
};
