/*
  ==============================================================================

    BaseGroup.h
    Created: 15 Apr 2025 2:07:26pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class T>
class ItemGroup : 
	public BaseItem
{
public:
	
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	ItemGroup(const juce::String& name)
		: BaseItem(name)
	{
	}

	virtual ~ItemGroup() {}
};