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

	InspectableEditor * getEditor(bool isRoot) override;
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
	BoolParameter * checkUpdatesOnStartup;
	BoolParameter * checkBetaUpdates;
	BoolParameter * updateHelpOnStartup;
	BoolParameter * openLastDocumentOnStartup;
	BoolParameter * openSpecificFileOnStartup;
	FileParameter * fileToOpenOnStartup;

	ControllableContainer interfaceCC;
	BoolParameter* closeToSystemTray;
	IntParameter* fontSize;
	EnumParameter* helpLanguage;

	ControllableContainer saveLoadCC;
	BoolParameter * enableAutoSave;
	IntParameter * autoSaveCount;
	IntParameter* autoSaveTime;

	BoolParameter* compressOnSave;
	BoolParameter* enableCrashUpload;

	ControllableContainer editingCC;
	BoolParameter * askBeforeRemovingItems;

	KeyMappingsContainer keyMappingsCC;

	Trigger * testCrash;

	void onControllableFeedbackUpdate(ControllableContainer *, Controllable * c) override;
	void loadJSONDataInternal(var data) override;

};