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
	public Component,
	public ComponentListener
{
public:
	InspectableEditor(Array<Inspectable *> inspectables, bool isRoot);
	InspectableEditor(Inspectable * inspectable, bool isRoot);

	virtual ~InspectableEditor();

	Array<WeakReference<Inspectable>> inspectables;
	WeakReference<Inspectable> inspectable;
    
    Inspector * parentInspector;

	bool fitToContent; 
	bool isRoot;
	bool isInsideInspectorBounds;


	void componentMovedOrResized(Component & c, bool wasMoved, bool wasResized) override;
	
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
	GenericComponentEditor(WeakReference<Inspectable> inspectable, Component * c, bool isRoot);
	~GenericComponentEditor();

	std::unique_ptr<Component> child;

	void resized() override;
	void childBoundsChanged(Component * c) override;
};