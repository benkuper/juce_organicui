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

void GenericControllableManager::addItemFromData(var data, bool fromUndoableAction)
{
	
}


GenericControllableManagerFactory::GenericControllableManagerFactory()
{
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Trigger", &GenericControllableItem::create)->addParam("controllableType", Trigger::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Float Parameter", &GenericControllableItem::create)->addParam("controllableType", FloatParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Int Parameter", &GenericControllableItem::create)->addParam("controllableType", IntParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Bool Parameter", &GenericControllableItem::create)->addParam("controllableType", BoolParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Color Parameter", &GenericControllableItem::create)->addParam("controllableType", ColorParameter::getTypeStringStatic()));
	defs.add(GenericControllableManagerFactory::Definition::createDef("", "Target Parameteer", &GenericControllableItem::create)->addParam("controllableType", TargetParameter::getTypeStringStatic()));
}