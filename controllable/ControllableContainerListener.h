#pragma once

class ControllableContainer;
class Controllable;

//Listener
class  ControllableContainerListener
{
public:
	/** Destructor. */
	virtual ~ControllableContainerListener() {}
	virtual void controllableAdded(Controllable *) {}
	virtual void controllableRemoved(Controllable *) {}
	virtual void controllableContainerAdded(ControllableContainer *) {}
	virtual void controllableContainerRemoved(ControllableContainer *) {}
	virtual void controllableFeedbackUpdate(ControllableContainer*, Controllable*) {}
	virtual void controllableStateUpdate(ControllableContainer *, Controllable *) {}
	virtual void childStructureChanged(ControllableContainer *) {}
	virtual void childAddressChanged(ControllableContainer *) {};
	virtual void controllableContainerNameChanged(ControllableContainer*) {}
	virtual void controllableContainerReordered(ControllableContainer *) {}
	virtual void controllableContainerFinishedLoading(ControllableContainer *) {};

};

// ASYNC
class  ContainerAsyncEvent {
public:
	enum EventType {
		ControllableAdded,
		ControllableRemoved,
		ControllableContainerAdded,
		ControllableContainerRemoved,
		ControllableFeedbackUpdate,
		ControllableStateUpdate,
		ChildStructureChanged,
		ChildAddressChanged,
		ControllableContainerPresetLoaded,
		ControllableContainerReordered,
		ControllableContainerNeedsRebuild,
		ControllableContainerCollapsedChanged,
		ControllableContainerFinishedLoading
	};

	ContainerAsyncEvent(EventType _type, ControllableContainer* _source) : type(_type), source(_source), targetContainer(nullptr), targetControllable(nullptr) {}
	ContainerAsyncEvent(EventType _type, ControllableContainer* _source, Controllable *_target) :
		type(_type), source(_source),
		targetContainer(nullptr),
		targetControllable(_target) {}
	ContainerAsyncEvent(EventType _type, ControllableContainer* _source, ControllableContainer *_target) : type(_type), source(_source), targetContainer(_target), targetControllable(nullptr) {}

	EventType type;
	WeakReference<ControllableContainer> source;
	WeakReference<ControllableContainer> targetContainer;
	WeakReference<Controllable> targetControllable;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ContainerAsyncEvent)
};

typedef QueuedNotifier<ContainerAsyncEvent>::Listener ContainerAsyncListener;