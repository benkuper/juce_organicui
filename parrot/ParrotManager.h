/*
  ==============================================================================

    ParrotManager.h
    Created: 4 Oct 2021 12:51:24pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParrotManager :
    public Manager<Parrot>
{
public:
    juce_DeclareSingleton(ParrotManager, true);

    ParrotManager();
    ~ParrotManager();
};