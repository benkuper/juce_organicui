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
	BoolParameter * launchMinimized;
#endif
	BoolParameter * checkUpdatesOnStartup;
	BoolParameter * checkBetaUpdates;
	BoolParameter * onlyCheckBetaFromBeta;
	BoolParameter * updateHelpOnStartup;
	BoolParameter * openLastDocumentOnStartup;
	BoolParameter * openSpecificFileOnStartup;
	StringParameter * fileToOpenOnStartup;

	ControllableContainer saveLoadCC;
	BoolParameter * enableAutoSave;
	IntParameter * autoSaveCount;

	ControllableContainer editingCC;
	BoolParameter * askBeforeRemovingItems;
	BoolParameter * constrainKeysToNeighbours;

	KeyMappingsContainer keyMappingsCC;

	void onControllableFeedbackUpdate(ControllableContainer *, Controllable * c) override;
	void loadJSONDataInternal(var data) override;

};