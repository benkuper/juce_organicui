/*
  ==============================================================================

	ControllableHelpers.cpp
	Created: 12 May 2016 4:21:18pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

//CONTROLLABLE
ControllableChooserPopupMenu::ControllableChooserPopupMenu(ControllableContainer* rootContainer, int _indexOffset, int _maxDefaultSearchLevel, const StringArray& typesFilter, const StringArray& excludeTypesFilter, std::function<bool(Controllable*)> filterFunc, Controllable* currentSelection) :
	indexOffset(_indexOffset),
	maxDefaultSearchLevel(_maxDefaultSearchLevel),
	typesFilter(typesFilter),
	excludeTypesFilter(excludeTypesFilter),
	filterFunc(filterFunc),
	currentSelection(currentSelection)
{
	int id = indexOffset + 1;

	if (rootContainer == nullptr) rootContainer = Engine::mainEngine;
	jassert(rootContainer != nullptr);
	populateMenu(this, rootContainer, id);
}

ControllableChooserPopupMenu::~ControllableChooserPopupMenu()
{
}

void ControllableChooserPopupMenu::populateMenu(PopupMenu* subMenu, ControllableContainer* container, int& currentId, int currentLevel)
{
	if (container == nullptr)
	{
		jassertfalse;//should not be here
		return;
	}

	if (maxDefaultSearchLevel == -1 || currentLevel < maxDefaultSearchLevel)
	{
		for (auto& cc : container->controllableContainers)
		{
			if (cc.wasObjectDeleted() || cc == nullptr)
			{
				jassertfalse;
				continue;
			}

			PopupMenu p;
			populateMenu(&p, cc, currentId, currentLevel + 1);
			subMenu->addSubMenu(cc->niceName, p);

		}

		subMenu->addSeparator();
	}

	for (auto& c : container->controllables)
	{
		if (c == nullptr) continue;

		if (excludeTypesFilter.contains(c->getTypeString())) continue;
		if (!typesFilter.isEmpty() && !typesFilter.contains(c->getTypeString())) continue;
		if (filterFunc != nullptr && !filterFunc(c)) continue;

		subMenu->addItem(currentId, c->niceName, true, c == currentSelection);
		controllableList.add(c);
		currentId++;
	}

}

void ControllableChooserPopupMenu::showAndGetControllable(std::function<void(Controllable*)> returnFunc, bool deleteAfter)
{
	showMenuAsync(PopupMenu::Options(), [this, returnFunc, deleteAfter](int result)
		{
			returnFunc(getControllableForResult(result));
			if (deleteAfter) delete this;
		}
	);
}

Controllable* ControllableChooserPopupMenu::getControllableForResult(int result)
{
	if (result <= indexOffset || (result - 1 - indexOffset) >= controllableList.size()) return nullptr;
	return controllableList[result - 1 - indexOffset];
}

//CONTAINER


ContainerChooserPopupMenu::ContainerChooserPopupMenu(ControllableContainer* rootContainer, int indexOffset, int maxSearchLevel, std::function<bool(ControllableContainer*)> typeCheckFunc, const StringArray& typesFilter, const StringArray& excludeTypeFilters, bool allowSelectAtAnyLevel, ControllableContainer* currentSelection) :
	indexOffset(indexOffset),
	maxDefaultSearchLevel(maxSearchLevel),
	typeCheckFunc(typeCheckFunc),
	typesFilter(typesFilter),
	excludeTypesFilter(excludeTypeFilters),
	allowSelectAtAnyLevel(allowSelectAtAnyLevel),
	currentSelection(currentSelection)
{
	int id = indexOffset + 1;

	if (rootContainer == nullptr) rootContainer = Engine::mainEngine;
	jassert(rootContainer != nullptr);
	populateMenu(this, rootContainer, id);
}

ContainerChooserPopupMenu::~ContainerChooserPopupMenu()
{

}

void ContainerChooserPopupMenu::populateMenu(PopupMenu* subMenu, ControllableContainer* container, int& currentId, int currentLevel)
{
	for (auto& cc : container->controllableContainers)
	{
		bool isATarget = false;
		bool lastLevel = currentLevel == maxDefaultSearchLevel || cc->controllableContainers.size() == 0;

		if (typeCheckFunc != nullptr) isATarget = typeCheckFunc(cc);
		else if (lastLevel) isATarget = true;

		bool isSelectable = true;
		if (BaseItem* bi = dynamic_cast<BaseItem*>(cc.get()))
		{
			if (excludeTypesFilter.contains(bi->getTypeString())) isSelectable = false;
			if (!typesFilter.isEmpty() && !typesFilter.contains(bi->getTypeString())) isSelectable = false;
		}
		else if (typesFilter.size() > 0) isSelectable = false; //if there are filtered types, then anything not a BaseItem is not a target

		if (isATarget)
		{
			if (!isSelectable) continue;
			containerList.add(cc);
			subMenu->addItem(currentId, cc->niceName, true, cc == currentSelection);
			currentId++;
		}
		else if (maxDefaultSearchLevel == -1 || currentLevel < maxDefaultSearchLevel)
		{
			/*if (cc->skipControllableNameInAddress)
			{
				populateMenu(subMenu, cc, currentId, currentLevel + 1);
			}
			else
			{*/
			PopupMenu p;

			int numItemsToCheck = 1; //if only precreated items, do not add to menu

			if (allowSelectAtAnyLevel && isSelectable)
			{
				containerList.add(cc);
				p.addItem(currentId, "Select", true, cc == currentSelection);
				currentId++;
				p.addSeparator();

				numItemsToCheck++;
			}

			populateMenu(&p, cc, currentId, currentLevel + 1);
			if (typeCheckFunc == nullptr || (p.containsAnyActiveItems() && p.getNumItems() >= numItemsToCheck)) subMenu->addSubMenu(cc->niceName, p, true, nullptr, cc == currentSelection);
			//}

		}
	}
}

void ContainerChooserPopupMenu::showAndGetContainer(std::function<void(ControllableContainer*)> returnFunc)
{
	showMenuAsync(PopupMenu::Options(), [this, returnFunc](int result)
		{
			returnFunc(getContainerForResult(result));
		}
	);

}

ControllableContainer* ContainerChooserPopupMenu::getContainerForResult(int result)
{
	if (result <= indexOffset || (result - 1 - indexOffset) >= containerList.size()) return nullptr;
	return containerList[result - 1 - indexOffset];
}



//Other helpers

int ControllableComparator::compareElements(Controllable* c1, Controllable* c2)
{
	return c1->niceName.compareIgnoreCase(c2->niceName);
}

void ControllableParser::createControllablesFromJSONObject(var data, ControllableContainer* container)
{
	if (data.isArray())
	{
		for (int i = 0; i < data.size(); ++i)
		{
			ControllableContainer* cc = container->getControllableContainerByName(String(i), true);
			if (cc == nullptr)
			{
				cc = new ControllableContainer(String(i));
				container->addChildControllableContainer(cc, true);
				cc->isRemovableByUser = true;
				cc->userCanAddControllables = true;
				cc->saveAndLoadRecursiveData = true;
				cc->saveAndLoadName = true;
			}
			createControllablesFromJSONObject(data[i], cc);
		}
		return;
	}

	if (!data.isObject()) return;

	DynamicObject* dataObject = data.getDynamicObject();
	if (dataObject == nullptr) return;

	NamedValueSet props = dataObject->getProperties();

	for (auto& p : props)
	{
		if (!p.name.isValid()) continue;

		if ((p.value.isObject() && !p.value.isArray()) || (p.value.isArray() && p.value.size() > 0 && p.value[0].isObject()))
		{
			ControllableContainer* cc = container->getControllableContainerByName(p.name.toString(), true);
			if (cc == nullptr)
			{
				cc = new ControllableContainer(p.name.toString());
				container->addChildControllableContainer(cc, true);
				cc->userCanAddControllables = true;
				cc->saveAndLoadRecursiveData = true;
				cc->saveAndLoadName = true;
			}

			createControllablesFromJSONObject(p.value, cc);
		}
		else
		{
			createControllableFromJSONObject(p.name.toString(), p.value, container);
		}
	}
}

void ControllableParser::createControllableFromJSONObject(StringRef cName, var data, ControllableContainer* container)
{
	Controllable* newC = container->getControllableByName(cName, true);
	if (newC == nullptr)
	{
		if (data.isBool()) newC = new BoolParameter(cName, cName, false);
		else if (data.isDouble()) newC = new FloatParameter(cName, cName, 0);
		else if (data.isInt()) newC = new IntParameter(cName, cName, 0);
		else if (data.isString() || data.isVoid()) newC = new StringParameter(cName, cName, "");
		else if (data.isArray() && data.size() > 0)
		{
			if (data.size() <= 4 && (data[0].isDouble() || data[0].isInt()))
			{
				if (data.size() == 1) newC = new FloatParameter(cName, cName, 0);
				else if (data.size() == 2) newC = new Point2DParameter(cName, cName);
				else if (data.size() == 3) newC = new Point3DParameter(cName, cName);
				else if (data.size() == 4) newC = new ColorParameter(cName, cName);
			}
			else
			{
				ControllableContainer* cc = container->getControllableContainerByName(cName, true);
				if (cc == nullptr)
				{
					cc = new ControllableContainer(cName);
					container->addChildControllableContainer(cc, true);
					cc->userCanAddControllables = true;
					cc->saveAndLoadRecursiveData = true;
					cc->saveAndLoadName = true;
				}

				for (int i = 0; i < data.size(); i++) createControllableFromJSONObject(String(i), data[i], cc);
			}
		}

		if (newC != nullptr)
		{
			newC->isCustomizableByUser = true;
			newC->isRemovableByUser = true;
			newC->isSavable = true;
			newC->saveValueOnly = false;
			container->addControllable(newC);
		}
	}

	if (newC != nullptr)
	{
		if (newC->type == Controllable::TRIGGER && (int)data != 0) ((Trigger*)newC)->trigger();
		else
		{
			Parameter* param = dynamic_cast<Parameter*>(newC);
			if (param != nullptr) param->setValue(data.isVoid() ? "" : data, false, true);
		}
	}

}
