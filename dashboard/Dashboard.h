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

	void itemAdded(DashboardItem*) override;
	void itemsAdded(Array<DashboardItem*>) override;
	void itemRemoved(DashboardItem*) override;
	void itemsRemoved(Array<DashboardItem*>) override;
	void parameterFeedback(var data) override; //from DashboardItemListener


	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	var getServerData();

	class  DashboardListener
	{
	public:
		/** Destructor. */
		virtual ~DashboardListener() {}
		virtual void parameterFeedback(var data) = 0;
		virtual void askForRefresh(Dashboard * d) = 0;
	};

	ListenerList<DashboardListener> dashboardListeners;
	void addDashboardListener(DashboardListener* newListener) { dashboardListeners.add(newListener); }
	void removeDashboardListener(DashboardListener* listener) { dashboardListeners.remove(listener); }

	DECLARE_ASYNC_EVENT(Dashboard, Dashboard, dashboard, { EDITING_UPDATE });
};