#pragma once


class DashboardInspectableItem :
	public DashboardItem
{
public:
	DashboardInspectableItem(Inspectable * item);
	~DashboardInspectableItem();

	juce::WeakReference<Inspectable> inspectable;
	juce::String inspectableGhostAddress;
	TargetParameter* target;
	bool settingInspectable;

	virtual void clearItem() override;

	virtual void setInspectable(Inspectable* i);
	virtual void setInspectableInternal(Inspectable* i) {}

	virtual void inspectableDestroyed(Inspectable* i) override;
    virtual void childStructureChanged(ControllableContainer* cc) override;

	virtual void ghostInspectable() {}
	virtual void checkGhost() {}

	virtual juce::var getJSONData(bool includeNonOverriden = false) override;
	virtual void loadJSONDataItemInternal(juce::var data) override;

	void onContainerParameterChangedInternal(Parameter* p) override;

	virtual juce::String getTypeString() const override { return "DashboardControllableItem"; }

	class  InspectableItemEvent
	{
	public:
		enum Type { INSPECTABLE_CHANGED };

		InspectableItemEvent(Type type, Inspectable* i) : type(type), inspectable(i) {}

		Type type;
		juce::WeakReference<Inspectable> inspectable;
	};



	QueuedNotifier<InspectableItemEvent> inspectableItemNotifier;
	typedef QueuedNotifier<InspectableItemEvent>::Listener ItemAsyncListener;

	void addAsyncInspectableItemListener(ItemAsyncListener* newListener) { inspectableItemNotifier.addListener(newListener); }
	void addAsyncCoalescedInspectableItemListener(ItemAsyncListener* newListener) { inspectableItemNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncInspectableItemListener(ItemAsyncListener* listener) { inspectableItemNotifier.removeListener(listener); }
};
