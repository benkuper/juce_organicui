/*
  ==============================================================================

    Dashboard.h
    Created: 19 Apr 2017 10:57:04pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class Dashboard :
	public BaseItem,
	public DashboardItemManager::ManagerListener,
	public DashboardFeedbackBroadcaster::FeedbackListener
{
public:
	Dashboard();
	~Dashboard();

	bool isBeingEdited;
	StringParameter* password;
	BoolParameter* unlockOnce;
	DashboardItemManager itemManager;

	void setIsBeingEdited(bool value);

	void itemAdded(BaseItem*) override;
	void itemsAdded(juce::Array<BaseItem*>) override;
	void itemRemoved(BaseItem*) override;
	void itemsRemoved(juce::Array<BaseItem*>) override;
	void parameterFeedback(juce::var data) override; //from DashboardItemListener
	void dashboardFeedback(juce::var data) override; //from DashboardItemListener


	juce::var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataInternal(juce::var data) override;

	juce::var getServerData();

	class  DashboardListener
	{
	public:
		/** Destructor. */
		virtual ~DashboardListener() {}
		virtual void parameterFeedback(juce::var data) = 0;
		virtual void dashboardFeedback(juce::var data) = 0;
		virtual void askForRefresh(Dashboard * d) = 0;
	};

	DECLARE_INSPECTACLE_SAFE_LISTENER(Dashboard, dashboard);

	DECLARE_ASYNC_EVENT(Dashboard, Dashboard, dashboard, { EDITING_UPDATE }, EVENT_ITEM_CHECK);
};