/*
  ==============================================================================

    DashboardItem.h
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/


#pragma once

class DashboardItem :
	public BaseItem,
	public Inspectable::InspectableListener
{
public:
	DashboardItem(Inspectable * _target = nullptr);
	virtual ~DashboardItem();

	virtual BaseItemMinimalUI<DashboardItem> * createUI() { jassertfalse; return nullptr; }

	/*
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
	*/
};