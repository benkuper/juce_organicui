#pragma once


class DashboardInspectableItem :
	public DashboardItem
{
public:
	DashboardInspectableItem(Inspectable * item);
	~DashboardInspectableItem();

	WeakReference<Inspectable> inspectable;
	String inspectableGhostAddress;

	virtual void clearItem() override;

	virtual void setInspectable(Inspectable* i);
	virtual void setInspectableInternal(Inspectable* i) {}

	virtual void inspectableDestroyed(Inspectable* i) override;
    virtual void childStructureChanged(ControllableContainer* cc) override;

	virtual void ghostInspectable() {}
	virtual void checkGhost() {}

	virtual var getJSONData() override;
	virtual void loadJSONDataItemInternal(var data) override;

	virtual String getTypeString() const override { return "DashboardControllableItem"; }

	class  InspectableItemEvent
	{
	public:
		enum Type { INSPECTABLE_CHANGED };

		InspectableItemEvent(Type type, Inspectable* i) : type(type), inspectable(i) {}

		Type type;
		WeakReference<Inspectable> inspectable;
	};



	QueuedNotifier<InspectableItemEvent> inspectableItemNotifier;
	typedef QueuedNotifier<InspectableItemEvent>::Listener ItemAsyncListener;

	void addAsyncInspectableItemListener(ItemAsyncListener* newListener) { inspectableItemNotifier.addListener(newListener); }
	void addAsyncCoalescedInspectableItemListener(ItemAsyncListener* newListener) { inspectableItemNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncInspectableItemListener(ItemAsyncListener* listener) { inspectableItemNotifier.removeListener(listener); }
};
