/*
  ==============================================================================

    GlobalSettings.h
    Created: 3 Jan 2018 3:52:13pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class GlobalSettings :
	public ControllableContainer
{
public:
	juce_DeclareSingleton(GlobalSettings, true);

	GlobalSettings();
	~GlobalSettings();

	//Startup
	ControllableContainer startupCC;
	BoolParameter * checkUpdatesOnStartup;
	BoolParameter * checkBetaUpdates;
	BoolParameter * onlyCheckBetaFromBeta;
	BoolParameter * updateHelpOnStartup;
	BoolParameter * openLastDocumentOnStartup;
	BoolParameter * openSpecificFileOnStartup;
	StringParameter * fileToOpenOnStartup;

	ControllableContainer confirmationsCC;
	BoolParameter * askBeforeRemovingItems;

	void controllableFeedbackUpdate(ControllableContainer *, Controllable * c) override;
	void loadJSONDataInternal(var data) override;
};