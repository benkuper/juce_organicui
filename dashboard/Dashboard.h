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
	public DashboardItem::DashboardItemListener
{
public:
	Dashboard();
	~Dashboard();

	bool isBeingEdited;
	DashboardItemManager itemManager;

	void itemAdded(DashboardItem*) override;
	void itemRemoved(DashboardItem*) override;
	void itemDataFeedback(var data) override; //from DashboardItemListener

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	var getServerData();

	class  DashboardListener
	{
	public:
		/** Destructor. */
		virtual ~DashboardListener() {}
		virtual void itemDataFeedback(var data) = 0;
	};

	ListenerList<DashboardListener> dashboardListeners;
	void addDashboardListener(DashboardListener* newListener) { dashboardListeners.add(newListener); }
	void removeDashboardListener(DashboardListener* listener) { dashboardListeners.remove(listener); }
};