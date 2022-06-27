/*
  ==============================================================================

    GlobalSettings.h
    Created: 3 Jan 2018 3:52:13pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class KeyMappingsContainer :
	public ControllableContainer
{
public:
	KeyMappingsContainer();
	~KeyMappingsContainer();

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	InspectableEditor * getEditorInternal(bool isRoot, Array<Inspectable*> inspectables = Array<Inspectable*>()) override;
};

class GlobalSettings :
	public ControllableContainer
{
public:
	juce_DeclareSingleton(GlobalSettings, true);

	GlobalSettings();
	~GlobalSettings();


	//Startup
	ControllableContainer startupCC;

#if JUCE_WINDOWS
	BoolParameter * launchOnStartup;
#endif
	BoolParameter* launchMinimised;
	BoolParameter* allowMultipleInstances;
	BoolParameter * checkUpdatesOnStartup;
	BoolParameter * checkBetaUpdates;
	BoolParameter * updateHelpOnStartup;
	BoolParameter * openLastDocumentOnStartup;
	BoolParameter * openSpecificFileOnStartup;
	FileParameter * fileToOpenOnStartup;
	BoolParameter* autoReopenFileOnCrash;

	ControllableContainer interfaceCC;
	BoolParameter* closeToSystemTray;
	BoolParameter* enableTooltips;
	IntParameter* fontSize;
	EnumParameter* helpLanguage;


	ControllableContainer saveLoadCC;
	BoolParameter * enableAutoSave;
	IntParameter * autoSaveCount;
	IntParameter* autoSaveTime;

	BoolParameter* compressOnSave;
	BoolParameter* enableCrashUpload;
	BoolParameter* saveLogsToFile;

	ControllableContainer editingCC;
	BoolParameter * askBeforeRemovingItems;
	EnumParameter* defaultEasing;

	KeyMappingsContainer keyMappingsCC;

	Trigger * testCrash;

	void onControllableFeedbackUpdate(ControllableContainer *, Controllable * c) override;
	void loadJSONDataInternal(var data) override;

};