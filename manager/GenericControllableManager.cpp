#include "GenericControllableManager.h"
/*
  ==============================================================================

    GenericControllableManager.cpp
    Created: 13 May 2017 2:31:43pm
    Author:  Ben

  ==============================================================================
*/

GenericControllableManagerFactory GenericControllableManager::factory;

GenericControllableManager::GenericControllableManager(const String & name) :
	BaseManager(name)
{
	managerFactory = &GenericControllableManager::factory;
	selectItemWhenCreated = false;
}

GenericControllableManager::~GenericControllableManager()
{
}

InspectableEditor * GenericControllableManager::getEditor(bool isRoot)
{
	BaseManagerUI<GenericControllableManager, GenericControllableItem,GenericControllableItemUI> * ui = new BaseManagerUI<GenericControllableManager, GenericControllableItem, GenericControllableItemUI>(niceName, this, false);
	ui->drawContour = true;
	ui->addExistingItems();
	return new GenericComponentEditor(this, ui, isRoot);
}

GenericControllableManagerFactory::GenericControllableManagerFactory()
{
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Trigger", &GenericControllableItem::create)->addParam("controllableType", Trigger::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Float Parameter", &GenericControllableItem::create)->addParam("controllableType", FloatParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Int Parameter", &GenericControllableItem::create)->addParam("controllableType", IntParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Bool Parameter", &GenericControllableItem::create)->addParam("controllableType", BoolParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Color Parameter", &GenericControllableItem::create)->addParam("controllableType", ColorParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Target Parameteer", &GenericControllableItem::create)->addParam("controllableType", TargetParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Target Parameteer", &GenericControllableItem::create)->addParam("controllableType", TargetParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Point2D Parameteer", &GenericControllableItem::create)->addParam("controllableType", Point2DParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Point3D Parameteer", &GenericControllableItem::create)->addParam("controllableType", Point3DParameter::getTypeStringStatic()));
}