#include "JuceHeader.h"

BaseManager::BaseManager(const juce::String& name) :
	EnablingControllableContainer(name, false),
	canHaveGroups(false),
	itemDataType(""),
	userCanAddItemsManually(true),
	selectItemWhenCreated(true),
	autoReorderOnAdd(true),
	isManipulatingMultipleItems(false),
	viewZoom(1),
	showItemsInEditor(true),
	snapGridMode(nullptr),
	showSnapGrid(nullptr),
	snapGridSize(nullptr),
	comparator(this)
{
	skipLabelInTarget = true; //by default manager label in targetParameter UI are not interesting
	nameCanBeChangedByUser = false;
}

BaseManager::~BaseManager()
{
}

int BaseManager::getItemIndex(BaseItem* item) const
{
	for (int i = 0; i < baseItems.size(); ++i)
	{
		if (baseItems[i] == item)
			return i;
	}
	return -1;
}

int BaseManager::getItemIndex(const String& name, bool searchNiceNameToo) const
{
	for (int i = 0; i < baseItems.size(); ++i)
	{
		if (baseItems[i]->shortName == name || (searchNiceNameToo && baseItems[i]->niceName == name))
			return i;
	}
	return -1;
}

bool BaseManager::hasItems(bool includeGroups, bool recursive) const
{
	if (includeGroups || !canHaveGroups) return !baseItems.isEmpty();

	for (int i = 0; i < baseItems.size(); ++i)
	{
		if (BaseItemGroup* g = dynamic_cast<BaseItemGroup*>(baseItems[i]))
		{
			if (recursive)
			{
				if (g->baseManager->hasItems(includeGroups, recursive))
					return true;
			}
		}
		else
		{
			return true;
		}
	}

	return false;
}

int BaseManager::getNumItems(bool includeGroups, bool recursive) const
{
	int result = 0;

	if (!canHaveGroups) return baseItems.size();

	for (auto& i : baseItems)
	{
		if (dynamic_cast<BaseItemGroup*>(i) != nullptr)
		{
			if (recursive)
			{
				result += ((BaseItemGroup*)i)->baseManager->getNumItems(includeGroups, recursive);
			}
			
			if (includeGroups)	result++;
		}
		else
		{
			result++;
		}

	}

	return result;
}

bool BaseManager::hasItem(BaseItem* item, bool recursive) const
{
	if (baseItems.contains(item)) return true;

	if (recursive)
	{
		for (auto& i : baseItems)
		{
			if (BaseItemGroup* g = dynamic_cast<BaseItemGroup*>(i))
			{
				if (g->baseManager->hasItem(item, recursive))
					return true;
			}
		}
	}

	return false;
}


void BaseManager::setHasGridOptions(bool hasGridOptions)
{
	if (hasGridOptions)
	{
		if (snapGridMode == nullptr)
		{
			snapGridMode = addBoolParameter("Snap Grid Mode", "If enabled, this will force moving objects snap to grid", false);
			showSnapGrid = addBoolParameter("Show Snap Grid", "If checked, this will show the snap grid", false);
			snapGridSize = addIntParameter("Snap Grid Size", "The size of the grid cells to snap to", 20, 4, 1000);
		}
	}
	else
	{
		if (snapGridMode != nullptr)
		{
			removeControllable(snapGridMode);
			removeControllable(showSnapGrid);
			removeControllable(snapGridSize);

			snapGridMode = nullptr;
			showSnapGrid = nullptr;
			snapGridSize = nullptr;
		}
	}
}

BaseManager::ManagerItemComparator::ManagerItemComparator(BaseManager* manager) : 
	m(manager),
	compareFunc(nullptr)
{
}

int BaseManager::ManagerItemComparator::compareElements(BaseItem* i1, BaseItem* i2)
{
	jassert(compareFunc != nullptr);
	return compareFunc(i1, i2);
}