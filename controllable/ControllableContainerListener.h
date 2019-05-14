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
	virtual void controllableFeedbackUpdate(ControllableContainer *, Controllable *) {}
	virtual void childStructureChanged(ControllableContainer *) {}
	virtual void childAddressChanged(ControllableContainer *) {};
	//virtual void controllableContainerPresetLoaded(ControllableContainer *) {}
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
		ChildStructureChanged,
		ChildAddressChanged,
		ControllableContainerPresetLoaded,
		ControllableContainerReordered,
		ControllableContainerNeedsRebuild,
		ControllableContainerFinishedLoading
	};

	ContainerAsyncEvent(EventType _type, ControllableContainer* _source) : type(_type), source(_source), targetContainer(nullptr), targetControllable(nullptr) {}
	ContainerAsyncEvent(EventType _type, ControllableContainer* _source, Controllable *_target) :
		type(_type), source(_source),
		targetContainer(nullptr),
		targetControllable(_target) {}
	ContainerAsyncEvent(EventType _type, ControllableContainer* _source, ControllableContainer *_target) : type(_type), source(_source), targetContainer(_target), targetControllable(nullptr) {}

	EventType type;
	ControllableContainer * source;
	ControllableContainer * targetContainer;
	Controllable * targetControllable;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ContainerAsyncEvent)
};

typedef QueuedNotifier<ContainerAsyncEvent>::Listener ContainerAsyncListener;