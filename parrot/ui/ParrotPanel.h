/*
  ==============================================================================

    ParrotPanel.h
    Created: 4 Oct 2021 12:53:40pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParrotManagerUI :
    public ManagerShapeShifterUI<ParrotManager, Parrot>
{
public:
    ParrotManagerUI();
    ~ParrotManagerUI();

    static ParrotManagerUI* create(const juce::String& name) { return new ParrotManagerUI(); }
};