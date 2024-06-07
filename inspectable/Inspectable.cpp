#include "JuceHeader.h"

Inspectable::Inspectable() :
	selectionManager(nullptr), //default nullptr will target main selectionManager
	isSelected(false),
	isSelectable(true),
    showInspectorOnSelect(true),
	hideInEditor(false),
	hideInRemoteControl(false),
	defaultHideInRemoteControl(false),
	isHighlighted(false),
	isPreselected(false),
	isBeingDestroyed(false),
    highlightLinkedInspectableOnSelect(true),
	saveCustomData(true),
    inspectableNotifier(10)
{
	setSelectionManager(nullptr);
}

Inspectable::~Inspectable()
{
	isBeingDestroyed = true;
	for (auto &i : linkedInspectables)
	{
		if (i.wasObjectDeleted()) continue;
		if (isHighlighted) i->setHighlighted(false);
		i->unregisterLinkedInspectable(this, false);
	}


	listeners.call(&InspectableListener::inspectableDestroyed, this);
	//inspectableNotifier.addMessage(new InspectableEvent(InspectableEvent::DESTROYED, this));

	masterReference.clear();
} 


String Inspectable::getHelpID()
{
	return helpID;
}

void Inspectable::selectThis(bool addToSelection, bool notify)
{
	if (selectionManager == nullptr) return;
	if (!isSelectable) return;
	if (isSelected) return;

	selectionManager->selectInspectable(this, !addToSelection, notify);
	//if (helpID.isNotEmpty() && HelpBox::getInstanceWithoutCreating() != nullptr) HelpBox::getInstance()->setPersistentData(helpID);
}

void Inspectable::deselectThis(bool notify)
{
	if (selectionManager == nullptr) return;
	if (!isSelectable) return;
	if (!isSelected) return;

	selectionManager->deselectInspectable(this, notify);
}

void Inspectable::setSelected(bool value)
{
	if (!isSelectable) return;
	if (value == isSelected) return;

	isSelected = value;
	isPreselected = false;

	if (highlightLinkedInspectableOnSelect) highlightLinkedInspectables(isSelected);

	setSelectedInternal(value);

	listeners.call(&InspectableListener::inspectableSelectionChanged, this);
	inspectableNotifier.addMessage(new InspectableEvent(InspectableEvent::SELECTION_CHANGED, this));
}

void Inspectable::setHighlighted(bool value)
{
	if (value == isHighlighted) return;
	isHighlighted = value;
	listeners.call(&InspectableListener::inspectableHighlightChanged, this);
	inspectableNotifier.addMessage(new InspectableEvent(InspectableEvent::HIGHLIGHT_CHANGED, this));
}

void Inspectable::highlightLinkedInspectables(bool value)
{
	for (auto &i : linkedInspectables)
	{
		if (!i.wasObjectDeleted()) i->setHighlighted(value);
	}
}

void Inspectable::registerLinkedInspectable(WeakReference<Inspectable> i, bool setAlsoInOtherInspectable)
{
	if (i.wasObjectDeleted() || i == nullptr) return;

	linkedInspectables.addIfNotAlreadyThere(i);
	if (setAlsoInOtherInspectable) i->registerLinkedInspectable(this, false);
}

void Inspectable::unregisterLinkedInspectable(WeakReference<Inspectable> i, bool setAlsoInOtherInspectable)
{
	if (i.wasObjectDeleted())
	{
		cleanLinkedInspectables();
		return;
	}

	linkedInspectables.removeAllInstancesOf(i);
	if (setAlsoInOtherInspectable && !i.wasObjectDeleted() && i != nullptr) i->unregisterLinkedInspectable(this, false);
}

void Inspectable::cleanLinkedInspectables()
{
	linkedInspectables.removeAllInstancesOf(nullptr);
}

void Inspectable::setSelectionManager(InspectableSelectionManager * _selectionManager)
{
	if (selectionManager == _selectionManager && selectionManager != nullptr) return;

	if (_selectionManager != nullptr)
	{
		selectionManager = _selectionManager;
	} else if (InspectableSelectionManager::mainSelectionManager != nullptr)
	{
		selectionManager = InspectableSelectionManager::mainSelectionManager;
	}

}

void Inspectable::setPreselected(bool value)
{
	if (!isSelectable) return;
	if (value == isPreselected) return;

	isPreselected = value;

	listeners.call(&InspectableListener::inspectablePreselectionChanged, this);
	inspectableNotifier.addMessage(new InspectableEvent(InspectableEvent::PRESELECTION_CHANGED, this));
}

void Inspectable::setSelectedInternal(bool)
{
	//to be overriden
}

InspectableEditor* Inspectable::getEditor(bool isRoot, Array<Inspectable *> inspectables)
{
	if (inspectables.size() == 0) inspectables.add(this);
	if (customGetEditorFunc != nullptr) return customGetEditorFunc(isRoot, inspectables);
	return getEditorInternal(isRoot, inspectables);
}
