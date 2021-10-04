/*
  ==============================================================================

    ParrotPanel.h
    Created: 4 Oct 2021 12:53:40pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParrotManagerUI :
    public BaseManagerShapeShifterUI<ParrotManager, Parrot, ParrotUI>
{
public:
    ParrotManagerUI();
    ~ParrotManagerUI();

    static ParrotManagerUI* create(const String& name) { return new ParrotManagerUI(); }
};