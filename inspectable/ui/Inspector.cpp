#include "Inspector.h"
/*
  ==============================================================================

    Inspector.cpp
    Created: 9 May 2016 6:41:38pm
    Author:  bkupe

  ==============================================================================
*/



Inspector::Inspector(InspectableSelectionManager * _selectionManager) :
	selectionManager(nullptr),
	currentInspectable(nullptr),
	currentEditor(nullptr)
{
	
	setSelectionManager(_selectionManager);

	vp.setScrollBarsShown(true, false);
	vp.setScrollOnDragEnabled(false);
	vp.setScrollBarThickness(10);
	addAndMakeVisible(vp);

	resized();
}



Inspector::~Inspector()
{
	if (selectionManager != nullptr) selectionManager->removeSelectionListener(this);
	else if(InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);

	clear();
}

void Inspector::setSelectionManager(InspectableSelectionManager * newSM)
{
	if (selectionManager != nullptr) selectionManager->removeSelectionListener(this);
	
	selectionManager = newSM;

	if (selectionManager == nullptr) selectionManager = InspectableSelectionManager::mainSelectionManager;
	if (selectionManager != nullptr) selectionManager->addSelectionListener(this);
}

void Inspector::resized()
{
 juce::Rectangle<int> r = getLocalBounds().reduced(3);
	vp.setBounds(r);
	r.removeFromRight(10);

	if (currentEditor != nullptr)
	{
		if (!currentEditor->fitToContent) r.setHeight(currentEditor->getHeight());
		currentEditor->setSize(r.getWidth(), r.getHeight()); 
	}
}

void Inspector::setCurrentInspectable(WeakReference<Inspectable> inspectable)
{
	if (!isEnabled()) return;

	if (inspectable == currentInspectable)
	{
		return;
	}

	if (currentInspectable != nullptr)
	{
		if (!currentInspectable.wasObjectDeleted())
		{
			currentInspectable->removeInspectableListener(this); 
			currentInspectable->setSelected(false);
		}

		if (currentEditor != nullptr)
		{
			vp.setViewedComponent(nullptr);
			currentEditor = nullptr;
		}
	}
	currentInspectable = inspectable;

	if (currentInspectable.get() != nullptr)
	{
		currentInspectable->setSelected(true);
		currentInspectable->addInspectableListener(this);
		currentEditor = currentInspectable->getEditor(true);
	}

	vp.setViewedComponent(currentEditor, false);
	resized();

	listeners.call(&InspectorListener::currentInspectableChanged, this);
}


void Inspector::clear()
{
	setCurrentInspectable(nullptr);
}

void Inspector::inspectableDestroyed(Inspectable * i)
{
	if (currentInspectable == i) setCurrentInspectable(nullptr);
}

void Inspector::inspectablesSelectionChanged()
{
	if (selectionManager->isEmpty())
	{
		setCurrentInspectable(nullptr);
		return;
	}

	Inspectable * newI = selectionManager->currentInspectables[0];
	if (!newI->showInspectorOnSelect) return;
	setCurrentInspectable(newI);
}

InspectorUI::InspectorUI(const String &name, InspectableSelectionManager * selectionManager) :
	ShapeShifterContentComponent(name),
	inspector(selectionManager)
{
	addAndMakeVisible(&inspector);

	helpID = "Inspector";
}

InspectorUI::~InspectorUI()
{
}

void InspectorUI::resized()
{
	inspector.setBounds(getLocalBounds());
}
