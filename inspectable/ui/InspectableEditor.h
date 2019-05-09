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
	InspectableEditor(WeakReference<Inspectable> inspectable, bool isRoot);
	virtual ~InspectableEditor();

	WeakReference<Inspectable> inspectable;
    
    Inspector * parentInspector;

	bool fitToContent; 
	bool isRoot;
	bool isInsideInspectorBounds;

	void componentMovedOrResized(Component & c, bool wasMoved, bool wasResized) override;
	
	void updateVisibility();

	virtual void parentHierarchyChanged() override;

	

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectableEditor)
};


class GenericComponentEditor :
	public InspectableEditor
{
public:
	GenericComponentEditor(WeakReference<Inspectable> inspectable, Component * c, bool isRoot);
	~GenericComponentEditor();

	ScopedPointer<Component> child;

	void resized() override;
	void childBoundsChanged(Component * c) override;
};