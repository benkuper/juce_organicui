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

class Inspectable :
	public WarningTarget
{
public:
	Inspectable();
	virtual ~Inspectable();

	InspectableSelectionManager * selectionManager;

	bool isSelected;
	bool isSelectable;
	bool showInspectorOnSelect;

	bool hideInEditor;
	bool hideInRemoteControl;
	bool defaultHideInRemoteControl;

	bool isHighlighted;

	//for multiSelection
	bool isPreselected;

	bool highlightLinkedInspectableOnSelect;
	Array<WeakReference<Inspectable>> linkedInspectables;

	//For storing arbitraty data
	var customData;
	bool saveCustomData;

	//editor
	std::function<InspectableEditor* (bool, Array<Inspectable*>)> customGetEditorFunc;

	//Help
	String helpID;
	virtual String getHelpID();


	virtual void selectThis(bool addToSelection = false, bool notify = true);
	virtual void deselectThis(bool notify = true);
	virtual void setSelected(bool value);

	virtual void setHighlighted(bool value);
	virtual void highlightLinkedInspectables(bool value);

	virtual void registerLinkedInspectable(WeakReference<Inspectable> i, bool setAlsoInOtherInspectable = true);
	virtual void unregisterLinkedInspectable(WeakReference<Inspectable> i, bool setAlsoInOtherInspectable = true);
	virtual void cleanLinkedInspectables();

	template<class T>
	void unregisterLinkedInspectablesOfType();

	virtual void setSelectionManager(InspectableSelectionManager * selectionManager);

	void setPreselected(bool value);

	virtual void setSelectedInternal(bool value); //to be overriden


	//Helper
	template<class S, class T>
	static Array<T*> getArrayAs(Array<S*> source)
	{
		Array<T*> result;
		for (auto& i : source) result.add(static_cast<T *>(i));
		return result;
	}

	template<class T>
	static Array<WeakReference<T>> getWeakArray(Array<T*> source)
	{
		Array<WeakReference<T>> result;
		for (auto& i : source) result.add(i);
		return result;
	}

	template<class T>
	static Array<T *> getArrayFromWeak(Array<WeakReference<T>> source)
	{
		Array<T *> result;
		for (auto& i : source) result.add(i.get());
		return result;
	}

	InspectableEditor* getEditor(bool isRoot, Array<Inspectable*> inspectables = Array<Inspectable*>());
	virtual InspectableEditor* getEditorInternal(bool /*isRoot*/, Array<Inspectable*> inspectables = Array<Inspectable*>()) { jassert(false);  return nullptr; } //to override !
	//Listener
	class  InspectableListener
	{
	public:
		/** Destructor. */
		virtual ~InspectableListener() {}
		virtual void inspectableSelectionChanged(Inspectable *) {};
		virtual void inspectablePreselectionChanged(Inspectable *) {};
		virtual void inspectableHighlightChanged(Inspectable *) {};

		virtual void inspectableDestroyed(Inspectable *) {};
	};

	ListenerList<InspectableListener> listeners;
	void addInspectableListener(InspectableListener* newListener) { listeners.add(newListener); }
	void removeInspectableListener(InspectableListener* listener) { listeners.remove(listener); }

	// ASYNC
	class  InspectableEvent
	{
	public:
		enum Type { SELECTION_CHANGED, PRESELECTION_CHANGED, HIGHLIGHT_CHANGED };

		InspectableEvent(Type t, Inspectable * inspectable) :
			type(t), inspectable(inspectable) {}

		Type type;
		Inspectable * inspectable;
	};

	QueuedNotifier<InspectableEvent> inspectableNotifier;
	typedef QueuedNotifier<InspectableEvent>::Listener AsyncListener;

	void addAsyncInspectableListener(AsyncListener* newListener) { inspectableNotifier.addListener(newListener); }
	void addAsyncCoalescedInspectableListener(AsyncListener* newListener) { inspectableNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncInspectableListener(AsyncListener* listener) { inspectableNotifier.removeListener(listener); }

	WeakReference<Inspectable>::Master masterReference;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Inspectable)
};


template<class T>
void Inspectable::unregisterLinkedInspectablesOfType()
{
	Array<Inspectable *> unregisterList;
	for (auto &i : linkedInspectables)
	{
		if (dynamic_cast<T *>(i.get()) != nullptr) unregisterList.add(i);
	}

	for (auto &i : unregisterList) unregisterLinkedInspectable(i);
	cleanLinkedInspectables();
}