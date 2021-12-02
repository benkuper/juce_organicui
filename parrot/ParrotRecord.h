/*
  ==============================================================================

    ParrotRecord.h
    Created: 4 Oct 2021 12:51:48pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParrotRecord :
    public BaseItem
{
public:
    ParrotRecord();
    ~ParrotRecord();
    
    double totalTime;

    struct RecordValue
    {
        RecordValue(double time = 0.0, var value = var()) : time(time), value(value) {}
        double time;
        var value;
    };

    void clearRecord();

    void addData(Controllable* c, double time, var value = var());

    HashMap<WeakReference<Controllable>, Array<RecordValue>> dataMap;

    var getJSONData() override;
    void loadJSONDataItemInternal(var data) override;

    String getTypeString() const override { return "Record"; }
};