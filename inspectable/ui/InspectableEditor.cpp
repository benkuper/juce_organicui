#include "InspectableEditor.h"
/*
  ==============================================================================

    CustomEditor.cpp
    Created: 9 May 2016 6:42:18pm
    Author:  bkupe

  ==============================================================================
*/


InspectableEditor::InspectableEditor(WeakReference<Inspectable> _inspectable, bool _isRoot) :
fitToContent(false),
    isRoot(_isRoot),
inspectable(_inspectable)
{
}


InspectableEditor::~InspectableEditor()
{
}






//GENERIC COMPONENT EDITOR

GenericComponentEditor::GenericComponentEditor(WeakReference<Inspectable> i, Component * c, bool isRoot) :
	InspectableEditor(i, isRoot),
	child(c)
{
	addAndMakeVisible(c);
	setSize(getWidth(), c->getHeight());
}


GenericComponentEditor::~GenericComponentEditor()
{
}

void GenericComponentEditor::resized()
{
	child->setBounds(getLocalBounds());
}

void GenericComponentEditor::childBoundsChanged(Component * c)
{
	setSize(getWidth(), child->getHeight());
}