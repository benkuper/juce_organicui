/*
  ==============================================================================

    DashboardItem.h
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

class DashboardItemUI;

#pragma once

class DashboardItem :
	public BaseItem,
	public Inspectable::InspectableListener
{
public:
	DashboardItem(Inspectable * _target = nullptr);
	virtual ~DashboardItem();

	WeakReference<Inspectable> target;
	virtual void setTarget(Inspectable * target);

	virtual void inspectableDestroyed(Inspectable * i);

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	virtual DashboardItemUI * createUI();

	class DashboardItemEvent
	{
	public:
		enum Type { TARGET_CHANGED };
		DashboardItemEvent(Type t) : type(t) {}
		Type type;
	};

	QueuedNotifier<DashboardItemEvent> dashboardItemNotifier;
	typedef QueuedNotifier<DashboardItemEvent>::Listener AsyncListener;

	void addAsyncDashboardItemListener(AsyncListener* newListener) { dashboardItemNotifier.addListener(newListener); }
	void addAsyncCoalescedDashboardItemListener(AsyncListener* newListener) { dashboardItemNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncDashboardItemListener(AsyncListener* listener) { dashboardItemNotifier.removeListener(listener); }
};