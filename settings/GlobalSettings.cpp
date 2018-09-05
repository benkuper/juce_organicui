/*
  ==============================================================================

    GlobalSettings.cpp
    Created: 3 Jan 2018 3:52:13pm
    Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(GlobalSettings)

GlobalSettings::GlobalSettings() :
	ControllableContainer("Global Settings"),
	startupCC("Startup and Update"),
	saveLoadCC("Save and Load"),
	editingCC("Editing")
{
	saveAndLoadRecursiveData = true;

	addChildControllableContainer(&startupCC);
	checkUpdatesOnStartup = startupCC.addBoolParameter("Check updates on startup", "If enabled, app will check if any updates are available",true);
	checkBetaUpdates = startupCC.addBoolParameter("Check for beta updates", "If enabled the app will also check for beta versions of the software", true);
	onlyCheckBetaFromBeta = startupCC.addBoolParameter("Only Check beta from beta versions", "If enabled the app will only check beta version when running a beta version itself", true);
	updateHelpOnStartup = startupCC.addBoolParameter("Update help on startup", "If enabled, app will try and download the last help file locally", true);
	
	openLastDocumentOnStartup = startupCC.addBoolParameter("Load last "+(Engine::mainEngine != nullptr?Engine::mainEngine->fileExtension:"")+" on startup", "If enabled, app will load the last " + Engine::mainEngine->fileExtension + " on startup", false);
	openSpecificFileOnStartup = startupCC.addBoolParameter("Load specific "+(Engine::mainEngine != nullptr?Engine::mainEngine->fileExtension:"")+" on startup", "If enabled, app will load the " + Engine::mainEngine->fileExtension + " specified below on startup", false,false);
	 
	fileToOpenOnStartup = new FileParameter("File to load on startup", "File to load when start, if the option above is checked", "", false);
	startupCC.addParameter(fileToOpenOnStartup);

	addChildControllableContainer(&saveLoadCC);
	enableAutoSave = saveLoadCC.addBoolParameter("Enable auto-save", "When enabled, a backup file will be saved every 5 min", true);
	autoSaveCount = saveLoadCC.addIntParameter("Auto-save count", "The number of different files to auto-save", 10, 1, 100);

	addChildControllableContainer(&editingCC);
	askBeforeRemovingItems = editingCC.addBoolParameter("Ask before removing items", "If enabled, you will get a confirmation prompt before removing any item", false);
	constrainKeysToNeighbours = editingCC.addBoolParameter("Constrain curve keys editing", "If enabled, keys won't be able to be moved past their neighbours when editing a curve", false);

	addChildControllableContainer(OSCRemoteControl::getInstance());
}

GlobalSettings::~GlobalSettings()
{
}

void GlobalSettings::controllableFeedbackUpdate(ControllableContainer *, Controllable * c)
{
	if (c == openLastDocumentOnStartup)
	{
		if (openLastDocumentOnStartup->boolValue()) openSpecificFileOnStartup->setValue(false);
		openSpecificFileOnStartup->setEnabled(!openLastDocumentOnStartup->boolValue());
	}else if (c == openLastDocumentOnStartup || c == openSpecificFileOnStartup)
	{
		fileToOpenOnStartup->setEnabled(openSpecificFileOnStartup->boolValue());
	}else if (c == checkBetaUpdates)
	{
		onlyCheckBetaFromBeta->setEnabled(checkBetaUpdates->boolValue());
	} else if (c == enableAutoSave)
	{
		autoSaveCount->setEnabled(enableAutoSave->boolValue());
	}
}

void GlobalSettings::loadJSONDataInternal(var data)
{
	openSpecificFileOnStartup->setEnabled(!openLastDocumentOnStartup->boolValue());
	fileToOpenOnStartup->setEnabled(openSpecificFileOnStartup->boolValue());
}
