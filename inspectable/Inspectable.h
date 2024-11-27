/*
  ==============================================================================

	Inspectable.h
	Created: 30 Oct 2016 9:02:24am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class InspectableEditor;
class InspectableSelectionManager;

#ifndef COMMA
#define COMMA ,
#endif

#define DECLARE_INSPECTACLE_LISTENER_BASE(ClassName, ListenerClass, listenerName, Section) ListenerClass<ClassName ## Listener Section> listenerName ## Listeners; \
void add ## ClassName ## Listener(ClassName ## Listener* newListener) { listenerName ## Listeners.add(newListener); } \
void remove ## ClassName ## Listener(ClassName ## Listener* listener) { if (isBeingDestroyed) return; listenerName ## Listeners.remove(listener); }

//#define DECLARE_INSPECTACLE_CRITICAL_LISTENER(ClassName, listenerName) DECLARE_INSPECTACLE_LISTENER_BASE(ClassName, juce::ListenerList, listenerName, COMMA juce::Array<ClassName ## Listener* COMMA juce::CriticalSection>)

#define DECLARE_INSPECTACLE_LISTENER(ClassName, listenerName) DECLARE_INSPECTACLE_LISTENER_BASE(ClassName, juce::ListenerList, listenerName,  )

#define DECLARE_INSPECTACLE_SAFE_LISTENER(ClassName, listenerName) DECLARE_INSPECTACLE_LISTENER_BASE(ClassName, juce::ThreadSafeBlockingListenerList, listenerName,)

#define DECLARE_INSPECTACLE_LIGHT_LISTENER(ClassName, listenerName) DECLARE_INSPECTACLE_LISTENER_BASE(ClassName, juce::LightweightListenerList, listenerName,)

class Inspectable :
	public WarningTarget
{
public:
	Inspectable();
	virtual ~Inspectable();

	InspectableSelectionManager* selectionManager;

	bool isSelected;
	bool isSelectable;
	bool showInspectorOnSelect;

	bool hideInEditor;
	bool hideInRemoteControl;
	bool defaultHideInRemoteControl;

	bool isHighlighted;

	//for multiSelection
	bool isPreselected;
	bool isBeingDestroyed;

	bool highlightLinkedInspectableOnSelect;
	juce::Array<juce::WeakReference<Inspectable>> linkedInspectables;

	//For storing arbitraty data
	juce::var customData;
	bool saveCustomData;

	//editor
	std::function<InspectableEditor* (bool, juce::Array<Inspectable*>)> customGetEditorFunc;

	//Help
	juce::String helpID;
	virtual juce::String getHelpID();


	virtual void selectThis(bool addToSelection = false, bool notify = true);
	virtual void deselectThis(bool notify = true);
	virtual void setSelected(bool value);

	virtual void setHighlighted(bool value, Inspectable* source = nullptr);
	virtual void highlightLinkedInspectables(bool value);

	virtual void registerLinkedInspectable(juce::WeakReference<Inspectable> i, bool setAlsoInOtherInspectable = true);
	virtual void unregisterLinkedInspectable(juce::WeakReference<Inspectable> i, bool setAlsoInOtherInspectable = true);
	virtual void cleanLinkedInspectables();

	template<class T>
	void unregisterLinkedInspectablesOfType();

	virtual void setSelectionManager(InspectableSelectionManager* selectionManager);

	void setPreselected(bool value);

	virtual void setSelectedInternal(bool value); //to be overriden


	//Helper
	template<class S, class T>
	static juce::Array<T*> getArrayAs(juce::Array<S*> source)
	{
		juce::Array<T*> result;
		for (auto& i : source) result.add(static_cast<T*>(i));
		return result;
	}

	template<class T>
	static juce::Array<juce::WeakReference<T>> getWeakArray(juce::Array<T*> source)
	{
		juce::Array<juce::WeakReference<T>> result;
		for (auto& i : source) result.add(i);
		return result;
	}

	template<class T>
	static juce::Array<T*> getArrayFromWeak(juce::Array<juce::WeakReference<T>> source)
	{
		juce::Array<T*> result;
		for (auto& i : source) result.add(i.get());
		return result;
	}

	InspectableEditor* getEditor(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>());
	virtual InspectableEditor* getEditorInternal(bool /*isRoot*/, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) { jassert(false);  return nullptr; } //to override !
	//Listener
	class  InspectableListener
	{
	public:
		/** Destructor. */
		virtual ~InspectableListener() {}
		virtual void inspectableSelectionChanged(Inspectable*) {};
		virtual void inspectablePreselectionChanged(Inspectable*) {};
		virtual void inspectableHighlightChanged(Inspectable*, Inspectable* /*source*/) {};

		virtual void inspectableDestroyed(Inspectable*) {};
	};

	DECLARE_INSPECTACLE_SAFE_LISTENER(Inspectable, inspectable);

	class InspectableEvent {
	public:
		enum Type { SELECTION_CHANGED, PRESELECTION_CHANGED, HIGHLIGHT_CHANGED };
		InspectableEvent(Type t, Inspectable* item, Inspectable* source = nullptr) : type(t), item(item), source(source) {}
		Type type;
		Inspectable* item;
		Inspectable* source;
	};

	QueuedNotifier<InspectableEvent> inspectableNotifier;

	typedef QueuedNotifier<InspectableEvent>::Listener AsyncListener; void addAsyncInspectableListener(AsyncListener* newListener) {
		inspectableNotifier.addListener(newListener);
	}
	void addAsyncCoalescedInspectableListener(AsyncListener* newListener) {
		inspectableNotifier.addAsyncCoalescedListener(newListener);
	}
	void removeAsyncInspectableListener(AsyncListener* listener) {
		if (!isBeingDestroyed) inspectableNotifier.removeListener(listener);
	};

	juce::WeakReference<Inspectable>::Master masterReference;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Inspectable)
};


template<class T>
void Inspectable::unregisterLinkedInspectablesOfType()
{
	juce::Array<Inspectable*> unregisterList;
	for (auto& i : linkedInspectables)
	{
		if (dynamic_cast<T*>(i.get()) != nullptr) unregisterList.add(i);
	}

	for (auto& i : unregisterList) unregisterLinkedInspectable(i);
	cleanLinkedInspectables();
}