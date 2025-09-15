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
	public juce::Thread
{
public:
	Parrot();
	~Parrot();

	ControllableContainer targetsCC;
	ParrotRecordManager recordManager;

	EnumParameter* currentRecordEnum;
	ParrotRecord* currentRecord;

	juce::HashMap<juce::WeakReference<Controllable>, int> dataMapPlayIndex;

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

	juce::Array<Controllable*> getAllTargets();

	void resetDataPlayIndices();

	void updateControls();

	TargetParameter* addTarget(Controllable* c);
	TargetParameter* getTargetForControllable(Controllable* c);
	void removeTargetForControllable(Controllable* c);

	void onContainerParameterChangedInternal(Parameter* p) override;
	void onContainerTriggerTriggered(Trigger* t) override;
	void onExternalTriggerTriggered(Trigger* t) override;
	void onExternalParameterValueChanged(Parameter* p) override;

	void itemAdded(BaseItem* r) override;
	void itemsAdded(juce::Array<BaseItem*> records) override;
	void itemRemoved(BaseItem* r) override;
	void itemsRemoved(juce::Array<BaseItem*> records) override;

	void startRecording();
	void stopRecording();
	void startPlaying();
	void pausePlaying();
	void stopPlaying();

	void run() override;

	void processControllables();
	void processControllable(Controllable* c, juce::var val);

	juce::var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataItemInternal(juce::var data) override;

	juce::String getTypeString() const override { return "Parrot"; }

	DECLARE_UI_FUNC;
};
