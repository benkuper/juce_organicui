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
	public juce::Component,
	public Inspectable::InspectableListener,
	public InspectableSelectionManager::AsyncListener
{
public:
	Inspector(InspectableSelectionManager* selectionManager = nullptr);
	virtual ~Inspector();

	InspectableSelectionManager* selectionManager;
	juce::Array<Inspectable*> currentInspectables;
	juce::Viewport vp;
	std::unique_ptr<InspectableEditor> currentEditor;

	bool curSelectionDoesNotAffectInspector;
	bool showTextOnEmpty;

	int tempScrollPosition;

	virtual void setSelectionManager(InspectableSelectionManager* newSM);
	virtual void setCurrentInspectables(juce::Array<Inspectable*>inspectables = juce::Array<Inspectable*>(), bool setInspectableSelection = true);

	virtual void paint(juce::Graphics& g) override;
	virtual void resized() override;
	virtual void resizedInternal(juce::Rectangle<int>& r);
	virtual void mouseDrag(const juce::MouseEvent& e) override;

	void storeScrollPosition();
	void restoreScrollPosition();

	virtual void clear();

	void inspectableDestroyed(Inspectable* inspectable) override;

	//From InspectableSelectionManager
	virtual void newMessage(const InspectableSelectionManager::SelectionEvent& e) override;

	virtual void onSelectionChanged();

	class  InspectorListener
	{
	public:
		virtual ~InspectorListener() {}
		virtual void currentInspectableChanged(Inspector*) {};
	};

	juce::ListenerList<InspectorListener> listeners;
	void addInspectorListener(InspectorListener* newListener) { listeners.add(newListener); }
	void removeInspectorListener(InspectorListener* listener) { listeners.remove(listener); }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Inspector)
};


class InspectorUI :
	public ShapeShifterContentComponent
{
public:
	InspectorUI(const juce::String& name, InspectableSelectionManager* selectionManager = nullptr);
	~InspectorUI();

	std::unique_ptr<Inspector> inspector;
	static std::function<Inspector* (InspectableSelectionManager*)> customCreateInspectorFunc;

	void resized() override;

	static InspectorUI* create(const juce::String& contentName) { return new InspectorUI(contentName); }
};