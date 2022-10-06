/*
  ==============================================================================

    DashboardIFrameItem.h
    Created: 5 Dec 2021 3:53:20pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class DashboardIFrameItem :
	public DashboardItem
{
public:
    DashboardIFrameItem(var params = var());
    ~DashboardIFrameItem();

	StringParameter* url;
	var getServerData() override;

	DashboardItemUI* createUI() override;

	String getTypeString() const override { return "IFrame"; }
};
