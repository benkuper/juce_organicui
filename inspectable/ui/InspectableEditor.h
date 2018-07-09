/*
  ==============================================================================

    InspectableEditor.h
    Created: 9 May 2016 6:42:18pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef INSPECTABLEEDITOR_H_INCLUDED
#define INSPECTABLEEDITOR_H_INCLUDED

class Inspectable;

class InspectableEditor : 
	public Component
{
public:
	InspectableEditor(WeakReference<Inspectable> inspectable, bool isRoot);
	virtual ~InspectableEditor();

	bool fitToContent; 
	bool isRoot;

	WeakReference<Inspectable> inspectable;

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


#endif  // INSPECTABLEEDITOR_H_INCLUDED
