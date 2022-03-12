/*
  ==============================================================================

    GenericControllableItem.cpp
    Created: 13 May 2017 2:43:46pm
    Author:  Ben

  ==============================================================================
*/


GenericControllableItem::GenericControllableItem(var params) :
	BaseItem("Item",true)
{
	typeAtCreation = params.getProperty("type", "");

	controllable = ControllableFactory::createControllable(params.getProperty("controllableType",""));
	controllable->description = "Custom control of type " + controllable->getTypeString();
	controllable->saveValueOnly = false;
	controllable->isCustomizableByUser = true;// controllable->type != Controllable::STRING;

	editorCanBeCollapsed = false;
	editorIsCollapsed = true;

	jassert(controllable != nullptr);

	isSelectable = false;

	//skipControllableNameInAddress = true; 
	addControllable(controllable);
	setNiceName(controllable->niceName);

}

GenericControllableItem::~GenericControllableItem()
{

}


void GenericControllableItem::onContainerNiceNameChanged()
{
	controllable->setNiceName(niceName);
}

void GenericControllableItem::controllableNameChanged(Controllable* c)
{
	BaseItem::controllableNameChanged(c);
	if (c == controllable) setNiceName(controllable->niceName);
}

InspectableEditor * GenericControllableItem::getEditorInternal(bool isRoot)
{
	return new GenericControllableItemEditor(this, isRoot);
}
