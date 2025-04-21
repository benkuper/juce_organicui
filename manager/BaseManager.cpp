#include "JuceHeader.h"

BaseManager::BaseManager(const juce::String& name, bool canHaveGroups) :
	EnablingControllableContainer(name, true),
	canHaveGroups(canHaveGroups),
	userCanAddItemsManually(true),
	selectItemWhenCreated(false),
	autoReorderOnAdd(false),
	isManipulatingMultipleItems(false),
	clipboardCopyOffset(0, 0),
	viewOffset(0, 0),
	viewZoom(1.0f),
	showItemsInEditor(true)
{
	canBeCopiedAndPasted = true;
}

juce::Array<BaseItem*> BaseManager::getBaseItems(bool recursive, bool includeDisabled, bool includeGroups) const
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
				if (group) group->baseManager->getBaseItems(recursive, includeGroups);
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
