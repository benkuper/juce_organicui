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
        RecordValue(double time = 0.0, juce::var value = juce::var()) : time(time), value(value) {}
        double time;
        juce::var value;
    };

    void clearRecord();

    void addData(Controllable* c, double time, juce::var value = juce::var());

    juce::HashMap<juce::WeakReference<Controllable>, juce::Array<RecordValue>> dataMap;

    juce::var getJSONData() override;
    void loadJSONDataItemInternal(juce::var data) override;

    juce::String getTypeString() const override { return "Record"; }
};