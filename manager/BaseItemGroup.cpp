/*
  ==============================================================================

    BaseGroup.cpp
    Created: 15 Apr 2025 2:07:26pm
    Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "ItemGroup.h"


BaseItemGroup::BaseItemGroup(const String& name, BaseManager* manager) :
	BaseItem(name),
	baseManager(manager)
{
	if (baseManager != nullptr)
	{
		addChildControllableContainer(baseManager);
	}
}

BaseItemGroup::~BaseItemGroup()
{
}
