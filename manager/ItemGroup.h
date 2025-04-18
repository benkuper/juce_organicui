/*
  ==============================================================================

	BaseGroup.h
	Created: 15 Apr 2025 2:07:26pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<typename T> class ItemGroup;
template<typename T, typename G> class Manager;

template<typename T>
class ItemGroup :
	public BaseItemGroup
{
public:
	ItemGroup(const juce::String& name) :
		manager("Items"),
		BaseItemGroup(name, &manager)
	{
	}

	virtual ~ItemGroup() {}

	Manager<T, ItemGroup<T>> manager;
};