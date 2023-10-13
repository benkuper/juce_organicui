/*
  ==============================================================================

    InspectableSelector.h
    Created: 25 Mar 2017 4:13:37pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class InspectableSelector : 
	public juce::Component
{
public:
	juce_DeclareSingleton(InspectableSelector, false);
	InspectableSelector();
	~InspectableSelector();

	juce::Component* selectablesParent;

	juce::Array<juce::Component*> selectables;
	juce::Array<Inspectable* > inspectables;

	juce::Rectangle<int> selectionBounds;

	//memory of current Selection
	InspectableSelectionManager* currentSelectionManager;
	bool clearSelectionAtEnd;

	void startSelection(juce::Component* parent, juce::Array<juce::Component*> selectables, InspectableSelectionManager* manager = nullptr, bool clearSelection = true);
	void startSelection(juce::Component* parent, juce::Array<juce::Component*> selectables, juce::Array<Inspectable*> relatedInspectables, InspectableSelectionManager* manager = nullptr, bool clearSelection = true); //If custom UI needed, use that function to provide different set of ui and inspectables (must be the same size !!)
	void endSelection(bool confirmSelection = true);

	void paint(juce::Graphics& g) override;

	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;


	class  SelectorListener
	{
	public:
		virtual ~SelectorListener() {}
		virtual void selectionEnded(juce::Array<juce::Component *> selectedComponents) {};
	};

	juce::ListenerList<SelectorListener> listeners;
	void addSelectorListener(SelectorListener* newListener) { listeners.add(newListener); }
	void removeSelectorListener(SelectorListener* listener) { listeners.remove(listener); }
	
};