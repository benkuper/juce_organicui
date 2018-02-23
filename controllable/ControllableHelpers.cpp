#include "ControllableHelpers.h"
/*
  ==============================================================================

    ControllableHelpers.cpp
    Created: 12 May 2016 4:21:18pm
    Author:  bkupe

  ==============================================================================
*/


//CONTROLLABLE

ControllableChooserPopupMenu::ControllableChooserPopupMenu(ControllableContainer * rootContainer, bool _showParameters, bool _showTriggers, int _indexOffset, int _maxDefaultSearchLevel) :
	indexOffset(_indexOffset),
	maxDefaultSearchLevel(_maxDefaultSearchLevel),
	showParameters(_showParameters),
	showTriggers(_showTriggers)
{
	int id = indexOffset + 1;
	populateMenu(this, rootContainer,id);
}

ControllableChooserPopupMenu::~ControllableChooserPopupMenu()
{
}

void ControllableChooserPopupMenu::populateMenu(PopupMenu * subMenu, ControllableContainer * container, int &currentId, int currentLevel)
{
	if (maxDefaultSearchLevel == -1 || currentLevel < maxDefaultSearchLevel)
	{
		for (auto &cc : container->controllableContainers)
		{
			if (!cc->isTargettable) continue;
			PopupMenu p;
			populateMenu(&p, cc, currentId,currentLevel+1);
			subMenu->addSubMenu(cc->niceName, p);
		}

		subMenu->addSeparator();
	}

	//if (subMenu != this)
	//{
	for (auto &c : container->controllables)
	{
		if (!c->isTargettable || !c->isControllableExposed) continue;

		if (c->type == Controllable::TRIGGER)
		{
			if (!showTriggers) continue;
		} else
		{
			if (!showParameters) continue;
		}

		subMenu->addItem(currentId, c->niceName);
		controllableList.add(c);
		currentId++;
	}
	//}

}

Controllable * ControllableChooserPopupMenu::showAndGetControllable()
{
	int result = show();
	return getControllableForResult(result);
}

Controllable * ControllableChooserPopupMenu::getControllableForResult(int result)
{
	if (result <= indexOffset || (result-1-indexOffset) >= controllableList.size()) return nullptr;
	return controllableList[result - 1 - indexOffset];
}

//CONTAINER


ContainerChooserPopupMenu::ContainerChooserPopupMenu(ControllableContainer * rootContainer, int indexOffset, int maxSearchLevel, std::function<bool(ControllableContainer*)> typeCheckFunc) :
	indexOffset(indexOffset),
	maxDefaultSearchLevel(maxSearchLevel),
	typeCheckFunc(typeCheckFunc)
{
	int id = indexOffset + 1;
	populateMenu(this, rootContainer, id);
}

ContainerChooserPopupMenu::~ContainerChooserPopupMenu()
{
	
}

void ContainerChooserPopupMenu::populateMenu(PopupMenu * subMenu, ControllableContainer * container, int & currentId, int currentLevel)
{
	for (auto &cc : container->controllableContainers)
	{
		if (!cc->isTargettable) continue;

		bool isATarget = (typeCheckFunc != nullptr)?isATarget = typeCheckFunc(cc):currentLevel == maxDefaultSearchLevel;
		if (isATarget)
		{
			containerList.add(cc);
			subMenu->addItem(currentId, cc->niceName);
			currentId++;
		} else if (maxDefaultSearchLevel == -1 || currentLevel < maxDefaultSearchLevel)
		{
			PopupMenu p;
			populateMenu(&p, cc, currentId, currentLevel + 1);
			if(typeCheckFunc == nullptr || p.containsAnyActiveItems()) subMenu->addSubMenu(cc->niceName, p);
		}
	}
}

ControllableContainer * ContainerChooserPopupMenu::showAndGetContainer()
{
	int result = show();
	return getContainerForResult(result);
}

ControllableContainer * ContainerChooserPopupMenu::getContainerForResult(int result)
{
	if (result <= indexOffset || (result - 1 - indexOffset) >= containerList.size()) return nullptr;
	return containerList[result - 1 - indexOffset];
}



//Other helpers

int ControllableComparator::compareElements(Controllable * c1, Controllable * c2)
{
	return c1->niceName.compareIgnoreCase(c2->niceName);
}

var ControllableUtil::createDataForParam(const String & type, const String & name, const String & description, var value, var minVal, var maxVal, bool editable, bool hiddenInEditor)
{
	var v = var(new DynamicObject());
	v.getDynamicObject()->setProperty("type", type);
	v.getDynamicObject()->setProperty("niceName", name);
	v.getDynamicObject()->setProperty("description", description);
	v.getDynamicObject()->setProperty("value", value);
	v.getDynamicObject()->setProperty("minValue", minVal);
	v.getDynamicObject()->setProperty("maxValue", maxVal);
	v.getDynamicObject()->setProperty("editable", editable);
	v.getDynamicObject()->setProperty("hiddenInEditor", hiddenInEditor);
	return v;
}



