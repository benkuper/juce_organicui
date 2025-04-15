/*
  ==============================================================================

    ParrotRecordManager.h
    Created: 4 Oct 2021 12:54:13pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParrotRecordManager :
    public Manager<ParrotRecord>
{
public:
    ParrotRecordManager();
    ~ParrotRecordManager();
};