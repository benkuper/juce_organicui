/*
  ==============================================================================

    GenericControllableManager.cpp
    Created: 13 May 2017 2:31:43pm
    Author:  Ben

  ==============================================================================
*/

#include "../manager/ui/GenericManagerEditor.h"

GenericControllableManager::GenericControllableManager(const String & name, bool itemsCanBeDisabled, bool canAddTriggers, bool canAddTargets, bool canAddEnums) :
	BaseManager(name),
	itemsCanBeDisabled(itemsCanBeDisabled),
	forceItemsFeedbackOnly(false)
{
	selectItemWhenCreated = false;
	isSelectable = false;

	managerFactory = &factory;

	if(canAddTriggers) factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Trigger", &GenericControllableItem::create)->addParam("controllableType", Trigger::getTypeStringStatic()));
	factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Float Parameter", &GenericControllableItem::create)->addParam("controllableType", FloatParameter::getTypeStringStatic()));
	factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Int Parameter", &GenericControllableItem::create)->addParam("controllableType", IntParameter::getTypeStringStatic()));
	factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Bool Parameter", &GenericControllableItem::create)->addParam("controllableType", BoolParameter::getTypeStringStatic()));
	factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "String Parameter", &GenericControllableItem::create)->addParam("controllableType", StringParameter::getTypeStringStatic()));
	factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Color Parameter", &GenericControllableItem::create)->addParam("controllableType", ColorParameter::getTypeStringStatic()));
	factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Point2D Parameter", &GenericControllableItem::create)->addParam("controllableType", Point2DParameter::getTypeStringStatic()));
	factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Point3D Parameter", &GenericControllableItem::create)->addParam("controllableType", Point3DParameter::getTypeStringStatic()));

	if (canAddEnums) factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Enum Parameter", &GenericControllableItem::create)->addParam("controllableType", EnumParameter::getTypeStringStatic()));

	if(canAddTargets) factory.defs.add(Factory<GenericControllableItem>::Definition::createDef("", "Target Parameter", &GenericControllableItem::create)->addParam("controllableType", TargetParameter::getTypeStringStatic()));

}

GenericControllableManager::~GenericControllableManager()
{
}

void GenericControllableManager::setForceItemsFeedbackOnly(bool value)
{
	forceItemsFeedbackOnly = value;
	for (auto &i : items) i->controllable->setControllableFeedbackOnly(forceItemsFeedbackOnly);
}

void GenericControllableManager::addItemInternal(GenericControllableItem * item, var)
{
	item->canBeDisabled = itemsCanBeDisabled;
	if(forceItemsFeedbackOnly) item->controllable->setControllableFeedbackOnly(true);
}

InspectableEditor * GenericControllableManager::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new GenericManagerEditor<GenericControllableItem>(this, isRoot);
	/*
	BaseManagerUI<GenericControllableManager, GenericControllableItem,GenericControllableItemUI> * ui = new BaseManagerUI<GenericControllableManager, GenericControllableItem, GenericControllableItemUI>(niceName, this, false);
	ui->drawContour = true;
	ui->addExistingItems();
	ui->setSize(100, 100);
	return new GenericComponentEditor(this, ui, isRoot);
	*/
}