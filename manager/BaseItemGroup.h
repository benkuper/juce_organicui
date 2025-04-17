/*
  ==============================================================================

	BaseGroup.h
	Created: 15 Apr 2025 2:07:26pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class BaseManager;

class BaseItemGroup :
	public BaseItem
{
public:
	BaseItemGroup(const juce::String& name = "Group", BaseManager* manager = nullptr);
	virtual ~BaseItemGroup();

	BaseManager* baseManager;
};