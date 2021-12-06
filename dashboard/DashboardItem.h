/*
  ==============================================================================

    DashboardItem.h
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/


#pragma once
class DashboardItemUI;

class DashboardItem :
	public BaseItem,
	public Inspectable::InspectableListener
{
public:
	DashboardItem(Inspectable* _target = nullptr, const String &name = "Item");
	virtual ~DashboardItem();

	virtual DashboardItemUI* createUI();

	virtual var getServerData();

	virtual void onContainerParameterChangedInternal(Parameter *) override;

	void notifyDataFeedback(var data);

	class  DashboardItemListener
	{
	public:
		/** Destructor. */
		virtual ~DashboardItemListener() {}
		virtual void itemDataFeedback(var data) = 0;
	};

	ListenerList<DashboardItemListener> dashboardItemsListeners;
	void addDashboardItemListener(DashboardItemListener* newListener) { dashboardItemsListeners.add(newListener); }
	void removeDashboardItemListener(DashboardItemListener* listener) { dashboardItemsListeners.remove(listener); }
};