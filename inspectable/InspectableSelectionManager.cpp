#include "InspectableSelectionManager.h"
/*
  ==============================================================================

    InspectableSelectionManager.cpp
    Created: 6 Feb 2017 12:40:22pm
    Author:  Ben

  ==============================================================================
*/


InspectableSelectionManager * InspectableSelectionManager::mainSelectionManager = nullptr;
InspectableSelectionManager * InspectableSelectionManager::activeSelectionManager = nullptr;

InspectableSelectionManager::InspectableSelectionManager(bool isMainSelectionManager) :
	selectionNotifier(100)
{
	if (isMainSelectionManager) InspectableSelectionManager::mainSelectionManager = this;
	if (InspectableSelectionManager::activeSelectionManager == nullptr) InspectableSelectionManager::activeSelectionManager = this;

    setEnabled(true);
}

InspectableSelectionManager::~InspectableSelectionManager()
{
	clearSelection(false);
	if (InspectableSelectionManager::mainSelectionManager == this)
	{
		InspectableSelector::deleteInstance();
		InspectableSelectionManager::mainSelectionManager = nullptr;
	}
}

void InspectableSelectionManager::setEnabled(bool value)
{
	enabled = value;
	if (!enabled) clearSelection();
}

void InspectableSelectionManager::selectInspectables(Array<Inspectable*> inspectables, bool doClearSelection, bool notify)
{
	if (!enabled) return;
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return;

	activeSelectionManager = this;

	if (doClearSelection) clearSelection(false);
	for (auto &i : inspectables) selectInspectable(i, false, false);
	if (notify)
	{
		listeners.call(&Listener::inspectablesSelectionChanged);
		selectionNotifier.addMessage(new SelectionEvent(SelectionEvent::SELECTION_CHANGED, this));
	}

}

void InspectableSelectionManager::selectInspectable(WeakReference<Inspectable> inspectable, bool doClearSelection, bool notify)
{
	if (!enabled) return;
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return;

	if (currentInspectables.contains(inspectable)) return;
	
	activeSelectionManager = this;

	if (doClearSelection) clearSelection(false);
	
	if (inspectable.get() != nullptr)
	{
		inspectable->setSelected(true);
		inspectable->addInspectableListener(this);
		currentInspectables.add(inspectable);
	}

	if (notify)
	{
		listeners.call(&Listener::inspectablesSelectionChanged);
		selectionNotifier.addMessage(new SelectionEvent(SelectionEvent::SELECTION_CHANGED, this));
	}
}

void InspectableSelectionManager::deselectInspectable(WeakReference<Inspectable> inspectable, bool notify)
{
	if (!inspectable.wasObjectDeleted())
	{
		inspectable->removeInspectableListener(this);
		inspectable->setSelected(false);
		currentInspectables.removeAllInstancesOf(inspectable.get());
	}
	
	if(notify)
	{
		listeners.call(&Listener::inspectablesSelectionChanged);
		selectionNotifier.addMessage(new SelectionEvent(SelectionEvent::SELECTION_CHANGED, this));
	}
}

void InspectableSelectionManager::clearSelection(bool notify)
{
	while(currentInspectables.size() > 0)
	{
		deselectInspectable(currentInspectables[0],false);
	}

	currentInspectables.clear();
	if (notify)
	{
		listeners.call(&Listener::inspectablesSelectionChanged);
		selectionNotifier.addMessage(new SelectionEvent(SelectionEvent::SELECTION_CHANGED, this));
	}
}

bool InspectableSelectionManager::isEmpty()
{
	return currentInspectables.size() == 0;
}

void InspectableSelectionManager::inspectableDestroyed(Inspectable * i)
{
	deselectInspectable(i);
}
