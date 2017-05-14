#include "GenericControllableItem.h"
/*
  ==============================================================================

    GenericControllableItem.cpp
    Created: 13 May 2017 2:43:46pm
    Author:  Ben

  ==============================================================================
*/


GenericControllableItem::GenericControllableItem(var params) :
	BaseItem("Item",false)
{
	controllable = ControllableFactory::createControllable(params.getProperty("controllableType",""));
	controllable->description = "Custom control of type " + controllable->getTypeString();

	jassert(controllable != nullptr);

	isSelectable = false;

	skipControllableNameInAddress = true;
	controllable->isCustomizableByUser = true;
	addControllable(controllable);
	setNiceName(controllable->niceName);
}

GenericControllableItem::~GenericControllableItem()
{

}

InspectableEditor * GenericControllableItem::getEditor(bool isRoot)
{
	return new GenericControllableItemEditor(this, isRoot);
}

void GenericControllableItem::onContainerNiceNameChanged()
{
	controllable->setNiceName(niceName);
}
