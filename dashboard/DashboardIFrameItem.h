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
	DashboardIFrameItem(juce::var params = juce::var());
	~DashboardIFrameItem();

	StringParameter* url;
	juce::var getServerData() override;

	virtual juce::var getItemParameterFeedback(Parameter* p) override;

	DECLARE_TYPE("IFrame")

	static DashboardIFrameItem* create(juce::var params) { return new DashboardIFrameItem(params); }
	DECLARE_UI_FUNC;
};
