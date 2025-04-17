#include "JuceHeader.h"
#include "BaseManager.h"

BaseManager::BaseManager(const juce::String& name) :
	EnablingControllableContainer(name, false),
	canHaveGroups(true)
{
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
	if (includeGroups) return !baseItems.isEmpty();

	for (int i = 0; i < baseItems.size(); ++i)
	{
		if (BaseItemGroup* g = dynamic_cast<BaseItemGroup*>(baseItems[i]))
		{
			if (recursive)
			{
				if (g->baseManager->hasItems(includeGroups, recursive))
					return true;
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}

int BaseManager::getNumItems(bool includeGroups, bool recursive) const
{
	int result = 0;

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
