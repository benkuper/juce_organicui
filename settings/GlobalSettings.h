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

	juce::var getJSONData() override;
	void loadJSONDataInternal(juce::var data) override;

	InspectableEditor * getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
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

	ControllableContainer interfaceCC;
	BoolParameter* closeToSystemTray;
	BoolParameter* enableTooltips;
	IntParameter* fontSize;
	EnumParameter* helpLanguage;
	BoolParameter* useGLRenderer;
	IntParameter* loggerRefreshRate;

	ControllableContainer saveLoadCC;
	BoolParameter * enableAutoSave;
	IntParameter * autoSaveCount;
	IntParameter* autoSaveTime;

	BoolParameter* compressOnSave;
	enum CrashAction { REPORT, KILL, REOPEN, RECOVER };
	EnumParameter* actionOnCrash;
	StringParameter* crashContactEmail;
	BoolParameter* saveLogsToFile;

	ControllableContainer editingCC;
	BoolParameter * askBeforeRemovingItems;
	FloatParameter* altScaleFactor;
	EnumParameter* defaultEasing;

	ControllableContainer launchArguments;

	KeyMappingsContainer keyMappingsCC;

	Trigger * testCrash;

	void onControllableFeedbackUpdate(ControllableContainer *, Controllable * c) override;
	void loadJSONDataInternal(juce::var data) override;

	void addLaunchArguments(const juce::String& commandLine, const CommandLineElements& elements);
};