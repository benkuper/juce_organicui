#include "JuceHeader.h"

juce::Array<BaseItem*> BaseManager::getAllItems(bool recursive, bool includeGroups, bool includeDisabled) const
{
	juce::Array<BaseItem*> result;
	for (auto& i : baseItems)
	{
		if (i->canBeDisabled && (i->enabled->boolValue() || includeDisabled)) continue;

		if (i->isGroup)
		{
			if (includeGroups) result.add((BaseItem*)i);
			if (recursive)
			{
				auto* group = (BaseItemGroup*)i;
				if (group) group->baseManager->getAllItems(recursive, includeGroups);
			}
		}
		else
		{
			result.add(i);
		}
	}

	return result;
}

void BaseManager::callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func)
{
	for (auto& i : baseItems)
	{
		if (i->canBeDisabled && (i->enabled->boolValue() || includeDisabled)) continue;

		if (i->isGroup)
		{
			if (includeGroups) func(i);
			if (recursive)
			{
				auto* group = (BaseItemGroup*)i;
				if (group) group->baseManager->callFunctionOnAllItems(recursive, includeGroups, includeDisabled, func);
			}
		}
		else
		{
			func(i);
		}
	}
}
