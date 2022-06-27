/*
  ==============================================================================

	GlobalSettings.cpp
	Created: 3 Jan 2018 3:52:13pm
	Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(GlobalSettings)

ApplicationCommandManager& getCommandManager();

GlobalSettings::GlobalSettings() :
	ControllableContainer("Global Settings"),
	startupCC("Startup and Update"),
	interfaceCC("Interface"),
	saveLoadCC("Save and Load"),
	editingCC("Editing")
{

	jassert(Engine::mainEngine != nullptr); //Must not call GlobalSettings::getInstance() before creating the engine !

	saveAndLoadRecursiveData = true;

#if JUCE_WINDOWS
	launchOnStartup = startupCC.addBoolParameter("Launch on system startup", "If checked, this app will automatically launch on system startup", false);
#endif

	launchMinimised = startupCC.addBoolParameter("Launch minimized", "If checked, this app will automatically minimized it self when launched", false);
	allowMultipleInstances = startupCC.addBoolParameter("Allow Multiple Instances", "If checked, it will be possible to launch multiple instances of this application at the same time (not working on Mac, you would have to actually duplicate the app)", false);
	checkUpdatesOnStartup = startupCC.addBoolParameter("Check updates on startup", "If enabled, app will check if any updates are available", true);
	checkBetaUpdates = startupCC.addBoolParameter("Check for beta updates", "If enabled the app will also check for beta versions of the software", false);
	updateHelpOnStartup = startupCC.addBoolParameter("Update help on startup", "If enabled, app will try and download the last help file locally", true);

	openLastDocumentOnStartup = startupCC.addBoolParameter("Load last " + (Engine::mainEngine != nullptr ? Engine::mainEngine->fileExtension : "") + " on startup", "If enabled, app will load the last " + Engine::mainEngine->fileExtension + " on startup", false);
	openSpecificFileOnStartup = startupCC.addBoolParameter("Load specific " + (Engine::mainEngine != nullptr ? Engine::mainEngine->fileExtension : "") + " on startup", "If enabled, app will load the " + Engine::mainEngine->fileExtension + " specified below on startup", false, false);
	fileToOpenOnStartup = new FileParameter("File to load on startup", "File to load when start, if the option above is checked", "", false);
	fileToOpenOnStartup->forceAbsolutePath = true;
	startupCC.addParameter(fileToOpenOnStartup);
	autoReopenFileOnCrash = startupCC.addBoolParameter("Auto Reopen crashed file", "If checked and a file was being edited while crashing, this will open the app and load a snapshot of the last session before the crash", false);

	addChildControllableContainer(&startupCC);

	closeToSystemTray = interfaceCC.addBoolParameter("Close to system tray", "If checked, closing the main window will remove the window from desktop and put it on the system tray, but the app will still be running", false);
	fontSize = interfaceCC.addIntParameter("Font size", "Global font size, may be altered in some cases but this is used as a reference", 14, 0, 30);
	enableTooltips = interfaceCC.addBoolParameter("Enable Tooltips", "If checked, this will show tooltips when mouse is over a parameter", true);
	helpLanguage = interfaceCC.addEnumParameter("Help language", "What language to download ? You will need to restart the software to see changes");
	helpLanguage->addOption("English", "en")->addOption("French", "fr")->addOption("Chinese", "cn");

	addChildControllableContainer(&interfaceCC);

	enableAutoSave = saveLoadCC.addBoolParameter("Enable auto-save", "When enabled, a backup file will be saved every 5 min", true);
	autoSaveCount = saveLoadCC.addIntParameter("Auto-save count", "The number of different files to auto-save", 10, 1, 100);
	autoSaveTime = saveLoadCC.addIntParameter("Auto-save time", "The time in minutes between two auto-saves (will)", 5, 1, 100);
	compressOnSave = saveLoadCC.addBoolParameter("Compress file", "If checked, the JSON content will be minified, otherwise it will be human-readable but larger size as well", true);
	enableCrashUpload = saveLoadCC.addBoolParameter("Enable Crash Upload", "If checked and a crashlog is found at startup, it will automatically upload it.\nThis crash log is a very small file but is immensely helpful for me, so please leave this option enabled unless you strongly feel like not helping me :)", true);
	testCrash = saveLoadCC.addTrigger("Test crash", "This will cause a crash, allowing for testing crashes. Don't push this unless you REALLY want to !!!");
	saveLogsToFile = saveLoadCC.addBoolParameter("Save logs", "If checked, the content of the Logger will be automatically saved to a file", false);
	addChildControllableContainer(&saveLoadCC);

	askBeforeRemovingItems = editingCC.addBoolParameter("Ask before removing items", "If enabled, you will get a confirmation prompt before removing any item", false);
	defaultEasing = editingCC.addEnumParameter("Default Easing", "Easing that is set by default when creating new automation keys");
	for (int i = 0; i < Easing::TYPE_MAX; i++) defaultEasing->addOption(Easing::typeNames[i], (Easing::Type)i, false);
	defaultEasing->defaultValue = Easing::typeNames[(int)Easing::BEZIER];
	defaultEasing->resetValue();

	addChildControllableContainer(&editingCC);
	addChildControllableContainer(OSCRemoteControl::getInstance());
	addChildControllableContainer(&keyMappingsCC);
}

GlobalSettings::~GlobalSettings()
{
}


void GlobalSettings::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	ControllableContainer::onControllableFeedbackUpdate(cc, c);

#if JUCE_WINDOWS	
	if (c == launchOnStartup)
	{

		String regKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\\" + OrganicApplication::getInstance()->getApplicationName();
		String regValue = "\"" + File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getFullPathName() + "\"";
		if (launchOnStartup->boolValue()) WindowsRegistry::setValue(regKey, regValue);
		else WindowsRegistry::deleteValue(regKey);
	}
#endif

	if (c == openLastDocumentOnStartup)
	{
		if (openLastDocumentOnStartup->boolValue()) openSpecificFileOnStartup->setValue(false);
		openSpecificFileOnStartup->setEnabled(!openLastDocumentOnStartup->boolValue());
	}
	else if (c == openLastDocumentOnStartup || c == openSpecificFileOnStartup)
	{
		fileToOpenOnStartup->setEnabled(openSpecificFileOnStartup->boolValue());
	}
	else if (c == enableAutoSave)
	{
		autoSaveCount->setEnabled(enableAutoSave->boolValue());
	}
	else if (c == helpLanguage)
	{
		HelpBox::getInstance()->loadHelp();
	}
	else if (c == testCrash)
	{
#if JUCE_DEBUG
		CrashDumpUploader::getInstance()->handleCrash(0); //win only
#else
		Controllable* crashC = nullptr;
		crashC->getJSONData(); //this will crash
#endif
	}
	else if (c == saveLogsToFile)
	{
		CustomLogger::getInstance()->setFileLogging(saveLogsToFile->boolValue());
	}

	if (Engine::mainEngine != nullptr) Engine::mainEngine->setChangedFlag(false); //force no need to save when changing something in global settings
}

void GlobalSettings::loadJSONDataInternal(var data)
{
	openSpecificFileOnStartup->setEnabled(!openLastDocumentOnStartup->boolValue());
	fileToOpenOnStartup->setEnabled(openSpecificFileOnStartup->boolValue());
}


KeyMappingsContainer::KeyMappingsContainer() :
	ControllableContainer("Key Mappings")
{
	editorIsCollapsed = true;
}

KeyMappingsContainer::~KeyMappingsContainer()
{

}

var KeyMappingsContainer::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	KeyPressMappingSet* kms = getCommandManager().getKeyMappings();
	std::unique_ptr<XmlElement> xmlElement(kms->createXml(true));
	String xmlData = xmlElement->toString();
	data.getDynamicObject()->setProperty("keyMappings", xmlData);
	return data;
}

void KeyMappingsContainer::loadJSONDataInternal(var data)
{
	ControllableContainer::loadJSONDataInternal(data);

	KeyPressMappingSet* kms = getCommandManager().getKeyMappings();
	std::unique_ptr<XmlElement> element = XmlDocument::parse(data.getProperty("keyMappings", "").toString());
	if (element != nullptr) kms->restoreFromXml(*element);
}

InspectableEditor* KeyMappingsContainer::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new KeyMappingsContainerEditor(this, isRoot);
}
