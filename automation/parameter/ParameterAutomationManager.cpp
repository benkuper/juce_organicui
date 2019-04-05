#include "ParameterAutomationManager.h"
/*
  ==============================================================================

	ParameterAutomation.cpp
	Created: 26 Apr 2018 3:17:25pm
	Author:  Ben

  ==============================================================================
*/

ParameterAutomationManager::ParameterAutomationManager(ControllableContainer * rootContainer) :
	BaseManager("Automations"),
	rootContainer(rootContainer)
{
	selectItemWhenCreated = false;
}

ParameterAutomationManager::~ParameterAutomationManager()
{
}

ParameterAutomation * ParameterAutomationManager::createItem()
{
	return new ParameterAutomation(rootContainer);
}
