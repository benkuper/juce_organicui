/*
  ==============================================================================

    Parrot.h
    Created: 4 Oct 2021 12:51:30pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Parrot :
    public BaseItem,
    public ParrotRecordManager::ManagerListener,
    public Thread
{
public:
    Parrot();
    ~Parrot();

    ControllableContainer targetsCC;
    ParrotRecordManager recordManager;

    EnumParameter* currentRecordEnum;
    ParrotRecord* currentRecord;

    HashMap<Controllable*, int> dataMapPlayIndex;

    Trigger* startRecordTrigger;
    Trigger* stopRecordTrigger;
    Trigger* playTrigger;
    Trigger* pauseTrigger;
    Trigger* stopTrigger;

    BoolParameter* forceValueAtStartRecord;
    BoolParameter* trimToFirstData;
    BoolParameter* trimToLastData;

    enum Status { IDLE, RECORDING, PLAYING };
    EnumParameter* status;
    FloatParameter* playProgression;
    BoolParameter* loop;

    double timeAtRecord;

    void setCurrentRecord(ParrotRecord* r);
    void updateRecordOptions();

    double getRelativeRecordTime();

    Array<Controllable*> getAllTargets();

    void resetDataPlayIndices();

    void updateControls();

    TargetParameter * addTarget(Controllable *c);
    TargetParameter* getTargetForControllable(Controllable* c);
    void removeTargetForControllable(Controllable* c);

    void onContainerParameterChangedInternal(Parameter* p) override;
    void onContainerTriggerTriggered(Trigger* t) override;
    void onExternalTriggerTriggered(Trigger* t) override;
    void onExternalParameterValueChanged(Parameter* p) override;

    void itemAdded(ParrotRecord* r) override;
    void itemRemoved(ParrotRecord* r) override;

    void startRecording();
    void stopRecording();
    void startPlaying();
    void pausePlaying();
    void stopPlaying();

    void run() override;
    
    void processControllables();
    void processControllable(Controllable* c, var val);

    var getJSONData();
    void loadJSONDataItemInternal(var data) override;

    String getTypeString() const override { return "Parrot"; }
};