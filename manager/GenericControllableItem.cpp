/*
  ==============================================================================

	GenericControllableItem.cpp
	Created: 13 May 2017 2:43:46pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

GenericControllableItem::GenericControllableItem(var params) :
	GenericControllableItem(nullptr, params)
{
}

GenericControllableItem::GenericControllableItem(Controllable* c, var params) :
	BaseItem("Item", true),
	controllable(c)
{
	typeAtCreation = params.getProperty("type", "");
	String cType = params.getProperty("controllableType", "");

	if (controllable == nullptr)
	{
		if (cType.isEmpty())
		{
			LOGERROR("Controllable item constuctor, type should not be empty");
			return;
		}

		controllable = ControllableFactory::createControllable(cType);
	}


	if (controllable == nullptr)
	{
		LOGERROR("Error creating controllable from type " << cType);
		return;
	}

	controllable->description = "Custom control of type " + controllable->getTypeString();
	controllable->saveValueOnly = false;
	controllable->isCustomizableByUser = true;// controllable->type != Controllable::STRING;
	controllable->userCanChangeName = true;

	editorCanBeCollapsed = false;
	editorIsCollapsed = true;

	jassert(controllable != nullptr);

	isSelectable = false;

	//skipControllableNameInAddress = true
	addControllable(controllable);
	setNiceName(controllable->niceName);

}

GenericControllableItem::~GenericControllableItem()
{

}


void GenericControllableItem::onContainerNiceNameChanged()
{
	if (controllable == nullptr) return;
	controllable->setNiceName(niceName);
}

void GenericControllableItem::controllableNameChanged(Controllable* c)
{
	BaseItem::controllableNameChanged(c);

	if (controllable == nullptr) return;
	if (c == controllable) setNiceName(controllable->niceName);
}

InspectableEditor* GenericControllableItem::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new GenericControllableItemEditor(this, isRoot);
}
