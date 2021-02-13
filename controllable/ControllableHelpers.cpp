/*
  ==============================================================================

	ControllableHelpers.cpp
	Created: 12 May 2016 4:21:18pm
	Author:  bkupe

  ==============================================================================
*/


//CONTROLLABLE

ControllableChooserPopupMenu::ControllableChooserPopupMenu(ControllableContainer * rootContainer, int _indexOffset, int _maxDefaultSearchLevel, const StringArray & typesFilter, const StringArray& excludeTypesFilter) :
	indexOffset(_indexOffset),
	maxDefaultSearchLevel(_maxDefaultSearchLevel),
	typesFilter(typesFilter),
	excludeTypesFilter(excludeTypesFilter)
{
	int id = indexOffset + 1;

	if (rootContainer == nullptr) rootContainer = Engine::mainEngine;
	jassert(rootContainer != nullptr);
	populateMenu(this, rootContainer, id);
}

ControllableChooserPopupMenu::~ControllableChooserPopupMenu()
{
}

void ControllableChooserPopupMenu::populateMenu(PopupMenu * subMenu, ControllableContainer * container, int &currentId, int currentLevel)
{
	if (container == nullptr)
	{
		jassertfalse;//should not be here
		return;
	}

	if (maxDefaultSearchLevel == -1 || currentLevel < maxDefaultSearchLevel)
	{
		for (auto &cc : container->controllableContainers)
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

	for (auto &c : container->controllables)
	{
		if (c == nullptr) continue;

		if (excludeTypesFilter.contains(c->getTypeString())) continue;
		if (!typesFilter.isEmpty() && !typesFilter.contains(c->getTypeString())) continue;

		subMenu->addItem(currentId, c->niceName);
		controllableList.add(c);
		currentId++;
	}

}

Controllable * ControllableChooserPopupMenu::showAndGetControllable()
{
	int result = show();
	return getControllableForResult(result);
}

Controllable * ControllableChooserPopupMenu::getControllableForResult(int result)
{
	if (result <= indexOffset || (result - 1 - indexOffset) >= controllableList.size()) return nullptr;
	return controllableList[result - 1 - indexOffset];
}

//CONTAINER


ContainerChooserPopupMenu::ContainerChooserPopupMenu(ControllableContainer * rootContainer, int indexOffset, int maxSearchLevel, std::function<bool(ControllableContainer*)> typeCheckFunc) :
	indexOffset(indexOffset),
	maxDefaultSearchLevel(maxSearchLevel),
	typeCheckFunc(typeCheckFunc)
{
	int id = indexOffset + 1;

	if (rootContainer == nullptr) rootContainer = Engine::mainEngine;
	jassert(rootContainer != nullptr);
	populateMenu(this, rootContainer, id);
}

ContainerChooserPopupMenu::~ContainerChooserPopupMenu()
{

}

void ContainerChooserPopupMenu::populateMenu(PopupMenu * subMenu, ControllableContainer * container, int & currentId, int currentLevel)
{
	for (auto &cc : container->controllableContainers)
	{
		bool isATarget = currentLevel == maxDefaultSearchLevel;
		if (typeCheckFunc != nullptr) isATarget |= typeCheckFunc(cc);
		else if (cc->controllableContainers.size() == 0) isATarget = true;

		if (isATarget)
		{
			containerList.add(cc);
			subMenu->addItem(currentId, cc->niceName);
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
				populateMenu(&p, cc, currentId, currentLevel + 1);
				if (typeCheckFunc == nullptr || p.containsAnyActiveItems()) subMenu->addSubMenu(cc->niceName, p);
			//}

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

