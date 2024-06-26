/*
  ==============================================================================

    ParrotManager.cpp
    Created: 4 Oct 2021 12:51:24pm
    Author:  bkupe

  ==============================================================================
*/

juce_ImplementSingleton(ParrotManager)

ParrotManager::ParrotManager() :
    BaseManager("Parrots")
{
    hideInRemoteControl = true;
    defaultHideInRemoteControl = true;
}

ParrotManager::~ParrotManager()
{
}
