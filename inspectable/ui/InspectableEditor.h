/*
  ==============================================================================

    InspectableEditor.h
    Created: 9 May 2016 6:42:18pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Inspectable;
class Inspector;

class InspectableEditor : 
	public juce::Component,
	public juce::ComponentListener
{
public:
	InspectableEditor(juce::Array<Inspectable *> inspectables, bool isRoot);
	InspectableEditor(Inspectable * inspectable, bool isRoot);

	virtual ~InspectableEditor();

	juce::Array<juce::WeakReference<Inspectable>> inspectables;
	juce::WeakReference<Inspectable> inspectable;
    
    Inspector * parentInspector;

	bool fitToContent; 
	bool isRoot;
	bool isInsideInspectorBounds;


	void componentMovedOrResized(juce::Component & c, bool wasMoved, bool wasResized) override;
	
	virtual void updateVisibility();

	virtual void parentHierarchyChanged() override;

	bool isMultiEditing() const { return inspectables.size() > 1; }

	template<typename Func>
	bool multiHasOneOf(Func f)
	{
		for (auto& i : inspectables) if (f(i)) return true;
		return false;
	}

	template<typename Func>
	bool multiHasAllOf(Func f)
	{
		for (auto& i : inspectables) if (!f(i)) return false;
		return true;
	}
	

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectableEditor)
};


class GenericComponentEditor :
	public InspectableEditor
{
public:
	GenericComponentEditor(juce::WeakReference<Inspectable> inspectable, juce::Component * c, bool isRoot);
	~GenericComponentEditor();

	std::unique_ptr<juce::Component> child;

	void resized() override;
	void childBoundsChanged(juce::Component * c) override;
};