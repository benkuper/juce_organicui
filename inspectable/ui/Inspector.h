/*
  ==============================================================================

    Inspector.h
    Created: 9 May 2016 6:41:38pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once


#include "../../ui/shapeshifter/ShapeShifterContent.h"
#include "InspectableEditor.h"
#include "../InspectableSelectionManager.h"

class Inspector :
	public Component,
	public Inspectable::InspectableListener,
	public InspectableSelectionManager::AsyncListener
{
public:
	Inspector(InspectableSelectionManager * selectionManager = nullptr);
	virtual ~Inspector();

	InspectableSelectionManager * selectionManager;
	WeakReference<Inspectable> currentInspectable;
	Viewport vp;
	std::unique_ptr<InspectableEditor> currentEditor;

	void setSelectionManager(InspectableSelectionManager * newSM);
	void resized() override;
	void setCurrentInspectable(WeakReference<Inspectable> inspectable);
	void clear();

    void inspectableDestroyed(Inspectable * inspectable) override;

	//From InspectableSelectionManager
	void newMessage(const InspectableSelectionManager::SelectionEvent &e) override;

	class  InspectorListener
	{
	public:
		virtual ~InspectorListener() {}
		virtual void currentInspectableChanged(Inspector *) {};
	};

	ListenerList<InspectorListener> listeners;
	void addInspectorListener(InspectorListener* newListener) { listeners.add(newListener); }
	void removeInspectorListener(InspectorListener* listener) { listeners.remove(listener); }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Inspector)
};


class InspectorUI :
	public ShapeShifterContentComponent
{
public:
	InspectorUI(const String &name, InspectableSelectionManager * selectionManager = nullptr);
	~InspectorUI();

	Inspector inspector;

	void resized() override;

	static InspectorUI * create(const String &contentName) { return new InspectorUI(contentName); }
};