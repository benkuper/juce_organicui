/*
  ==============================================================================

    ParameterAutomation.h
    Created: 26 Apr 2018 3:17:25pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ParameterAutomationManager :
	public BaseManager<ParameterAutomation>
{
public:
	ParameterAutomationManager(ControllableContainer * rootContainer);
	~ParameterAutomationManager();

	ControllableContainer * rootContainer;

	ParameterAutomation* createItem();
};