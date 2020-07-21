/*
  ==============================================================================

    WakeOnLan.h
    Created: 27 Oct 2016 2:09:50pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class WakeOnLan
{
public:
    static int wake(MACAddress* macAddress);
};