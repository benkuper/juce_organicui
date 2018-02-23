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
	confirmationsCC("Confirmation messages")
{
	saveAndLoadRecursiveData = true;

	addChildControllableContainer(&startupCC);
	checkUpdatesOnStartup = startupCC.addBoolParameter("Check updates on startup", "If enabled, app will check if any updates are available",true);
	checkBetaUpdates = startupCC.addBoolParameter("Check for beta updates", "If enabled the app will also check for beta versions of the software", true);
	onlyCheckBetaFromBeta = startupCC.addBoolParameter("Only Check beta from beta versions", "If enabled the app will only check beta version when running a beta version itself", true);
	updateHelpOnStartup = startupCC.addBoolParameter("Update help on startup", "If enabled, app will try and download the last help file locally", true);
	openLastDocumentOnStartup = startupCC.addBoolParameter("Load last noisette on startup", "If enabled, app will load the last noisette on startup", false);
	openSpecificFileOnStartup = startupCC.addBoolParameter("Load specific noisette on startup", "If enabled, app will load the noisette specified below on startup", false,false);
	
	fileToOpenOnStartup = new FileParameter("File to load on startup", "File to load when start, if the option above is checked", "", false);
	startupCC.addParameter(fileToOpenOnStartup);

	addChildControllableContainer(&confirmationsCC);
	askBeforeRemovingItems = confirmationsCC.addBoolParameter("Ask before removing items", "If enabled, you will get a confirmation prompt before removing any item", false);

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
	}
	if (c == openLastDocumentOnStartup || c == openSpecificFileOnStartup)
	{
		fileToOpenOnStartup->setEnabled(openSpecificFileOnStartup->boolValue());
	}if (c == checkBetaUpdates)
	{
		onlyCheckBetaFromBeta->setEnabled(checkBetaUpdates->boolValue());
	}
}

void GlobalSettings::loadJSONDataInternal(var data)
{
	openSpecificFileOnStartup->setEnabled(!openLastDocumentOnStartup->boolValue());
	fileToOpenOnStartup->setEnabled(openSpecificFileOnStartup->boolValue());
}
