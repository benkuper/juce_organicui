/*
  ==============================================================================

	GenericControllableManager.cpp
	Created: 13 May 2017 2:31:43pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

GenericControllableManager::GenericControllableManager(const String& name, bool itemsCanBeDisabled, bool canAddTriggers, bool canAddTargets, bool canAddEnums) :
	Manager(name),
	itemsCanBeDisabled(itemsCanBeDisabled),
	forceItemsFeedbackOnly(false)
{
	selectItemWhenCreated = false;
	isSelectable = false;

	managerFactory = &factory;

	if (canAddTriggers) factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Trigger")->addParam("controllableType", Trigger::getTypeStringStatic()));
	factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Float Parameter")->addParam("controllableType", FloatParameter::getTypeStringStatic()));
	factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Int Parameter")->addParam("controllableType", IntParameter::getTypeStringStatic()));
	factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Bool Parameter")->addParam("controllableType", BoolParameter::getTypeStringStatic()));
	factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "String Parameter")->addParam("controllableType", StringParameter::getTypeStringStatic()));
	factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Color Parameter")->addParam("controllableType", ColorParameter::getTypeStringStatic()));
	factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Point2D Parameter")->addParam("controllableType", Point2DParameter::getTypeStringStatic()));
	factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Point3D Parameter")->addParam("controllableType", Point3DParameter::getTypeStringStatic()));

	if (canAddEnums) factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Enum Parameter")->addParam("controllableType", EnumParameter::getTypeStringStatic()));

	if (canAddTargets) factory.defs.add(Factory::Definition::createDef<GenericControllableItem>("", "Target Parameter")->addParam("controllableType", TargetParameter::getTypeStringStatic()));

}

GenericControllableManager::~GenericControllableManager()
{
}

void GenericControllableManager::setForceItemsFeedbackOnly(bool value)
{
	forceItemsFeedbackOnly = value;
	callFunctionOnItemsOnly(false, false, true, [this](GenericControllableItem* i) { i->controllable->setControllableFeedbackOnly(forceItemsFeedbackOnly); });
}

GenericControllableItem* GenericControllableManager::addItemFrom(Controllable* sourceC, bool copyValue)
{
	Controllable* c = nullptr;
	if (Parameter* sourceP = dynamic_cast<Parameter*>(sourceC))
	{
		c = ControllableFactory::createParameterFrom(sourceP, true, true);
	}
	else
	{
		c = new Trigger(sourceC->niceName, sourceC->description);
	}

	if (c == nullptr) return nullptr;
	var params = new DynamicObject();
	String cType = getTypeForControllableType(c->getTypeString());
	params.getDynamicObject()->setProperty("type", cType);
	GenericControllableItem* gci = new GenericControllableItem(c, params);
	addItem(gci);
	return gci;
}

void GenericControllableManager::addItemInternal(BaseItem* item, var)
{
	item->canBeDisabled = itemsCanBeDisabled;
	if (forceItemsFeedbackOnly) getAsItem(item)->controllable->setControllableFeedbackOnly(true);
}


String GenericControllableManager::getTypeForControllableType(const String& type)
{
	for (auto& d : factory.defs)
	{
		if (Factory::Definition* fd = dynamic_cast<Factory::Definition*>(d))
		{
			if (fd->params.getProperty("controllableType", "") == type) return fd->type;
		}
	}
	return String();
}

InspectableEditor* GenericControllableManager::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new GenericManagerEditor<GenericControllableItem>(this, isRoot);
	/*
	ManagerUI<GenericControllableManager, GenericControllableItem,GenericControllableItemUI> * ui = new ManagerUI<GenericControllableManager, GenericControllableItem, GenericControllableItemUI>(niceName, this, false);
	ui->drawContour = true;
	ui->addExistingItems();
	ui->setSize(100, 100);
	return new GenericComponentEditor(this, ui, isRoot);
	*/
}