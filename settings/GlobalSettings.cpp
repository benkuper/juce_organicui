/*
  ==============================================================================

	GlobalSettings.cpp
	Created: 3 Jan 2018 3:52:13pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"
#include "GlobalSettings.h"

juce_ImplementSingleton(GlobalSettings)

ApplicationCommandManager& getCommandManager();

GlobalSettings::GlobalSettings() :
	ControllableContainer("Global Settings"),
	startupCC("Startup and Update"),
	interfaceCC("Interface"),
	saveLoadCC("Save and Load"),
	editingCC("Editing"),
	launchArguments("Launch Arguments")
{

	jassert(Engine::mainEngine != nullptr); //Must not call GlobalSettings::getInstance() before creating the engine !

	saveAndLoadRecursiveData = true;
	hideInRemoteControl = true;
	defaultHideInRemoteControl = true;

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


	addChildControllableContainer(&startupCC);

	closeToSystemTray = interfaceCC.addBoolParameter("Close to system tray", "If checked, closing the main window will remove the window from desktop and put it on the system tray, but the app will still be running", false);
	fontSize = interfaceCC.addIntParameter("Font size", "Global font size, may be altered in some cases but this is used as a reference", 14, 0, 30);
	enableTooltips = interfaceCC.addBoolParameter("Enable Tooltips", "If checked, this will show tooltips when mouse is over a parameter", true);
	helpLanguage = interfaceCC.addEnumParameter("Help language", "What language to download ? You will need to restart the software to see changes");
	helpLanguage->addOption("English", "en")->addOption("French", "fr")->addOption("Chinese", "cn");

	bool useGL = true;
#if JUCE_MAC || JUCE_LINUX //OpenGL is not working so well on mac and linux
	useGL = false;
#endif

	useGLRenderer = interfaceCC.addBoolParameter("Use OpenGL Renderer", "If checked, this will use hardware acceleration to render the interface. You may want to NOT use this on some platform or when using the IFrame Dashboard item. You need to restart if you change it.", useGL);
	
	uiRefreshRate = interfaceCC.addIntParameter("UI Refresh Rate", "The refresh rate of the UI in hz", 30, 1, 100);
	loggerRefreshRate = interfaceCC.addIntParameter("Logger Refresh Rate", "The refresh rate of the logger in hz", 20, 1, 1000);

	addChildControllableContainer(&interfaceCC);

	enableAutoSave = saveLoadCC.addBoolParameter("Enable auto-save", "When enabled, a backup file will be saved every 5 min", true);
	autoSaveCurrentFile = saveLoadCC.addBoolParameter("Auto-save current file", "If checked, the current file will be saved as well when auto-saving", false);
	autoSaveOnChangeOnly = saveLoadCC.addBoolParameter("Auto-save on change only", "If checked, the auto-save will only save when a change is detected", true);
	autoAskRestore = saveLoadCC.addBoolParameter("Ask to restore on startup", "If checked, the app will ask to restore the auto-saved file on startup if a more recent auto-saved file has been found", true);
	autoSaveCount = saveLoadCC.addIntParameter("Auto-save count", "The number of backup files to auto-save", 10, 1, 100);
	autoSaveTime = saveLoadCC.addIntParameter("Auto-save time", "The time in minutes between two auto-saves (will)", 5, 1, 100);
	compressOnSave = saveLoadCC.addBoolParameter("Compress file", "If checked, the JSON content will be minified, otherwise it will be human-readable but larger size as well", true);
	logAutosave = saveLoadCC.addBoolParameter("Log auto-save", "If checked, the auto-save will be logged in the logger", true);

	actionOnCrash = saveLoadCC.addEnumParameter("Action On Crash", "This determines what to do on a crash. Default shows the crash report window");
	actionOnCrash->addOption("Report", REPORT)->addOption("Kill", KILL)->addOption("Reopen", REOPEN)->addOption("Recover", RECOVER);
	autoSendCrashLog = saveLoadCC.addBoolParameter("Auto send crash log", "If checked and action on crash is not Report, the crash log will be automatically sent to the developer team", true);

	crashContactEmail = saveLoadCC.addStringParameter("Crash Contact Mail", "A mail address to use if you wish to be contacted by the developer team", "");

	testCrash = saveLoadCC.addTrigger("Test crash", "This will cause a crash, allowing for testing crashes. Don't push this unless you REALLY want to !!!");
	saveLogsToFile = saveLoadCC.addBoolParameter("Save logs", "If checked, the content of the Logger will be automatically saved to a file", false);
	addChildControllableContainer(&saveLoadCC);

	askBeforeRemovingItems = editingCC.addBoolParameter("Ask before removing items", "If enabled, you will get a confirmation prompt before removing any item", false);

	altScaleFactor = editingCC.addFloatParameter("Alt Scale factor", "Scale factor for editing sliders with alt", 0.5, 0, 1);

	defaultEasing = editingCC.addEnumParameter("Default Easing", "Easing that is set by default when creating new automation keys");
	for (int i = 0; i < Easing::TYPE_MAX; i++) defaultEasing->addOption(Easing::typeNames[i], (Easing::Type)i, false);
	defaultEasing->defaultValue = Easing::typeNames[(int)Easing::BEZIER];
	defaultEasing->resetValue();

	addChildControllableContainer(&editingCC);
	addChildControllableContainer(OSCRemoteControl::getInstance());
	addChildControllableContainer(&keyMappingsCC);

	launchArguments.includeInRecursiveSave = false;
	addChildControllableContainer(&launchArguments);
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
	else if (c == uiRefreshRate)
	{
		OrganicUITimers::getInstance()->setupTimers();
	}


	if (Engine::mainEngine != nullptr) Engine::mainEngine->setChangedFlag(false); //force no need to save when changing something in global settings
}

void GlobalSettings::loadJSONDataInternal(var data)
{
	openSpecificFileOnStartup->setEnabled(!openLastDocumentOnStartup->boolValue());
	fileToOpenOnStartup->setEnabled(openSpecificFileOnStartup->boolValue());
}

void GlobalSettings::loadKeyMappingsFromData()
{

	KeyPressMappingSet* kms = getCommandManager().getKeyMappings();
	std::unique_ptr<XmlElement> element = XmlDocument::parse(keyMappingsData.toString());
	if (element != nullptr) kms->restoreFromXml(*element);
}

void GlobalSettings::addLaunchArguments(const String& commandLine, const CommandLineElements& elements)
{
	launchArguments.addStringParameter("Command Line", "Full commandline", commandLine);

	for (auto& c : elements)
	{
		if (c.args.size() < 2) launchArguments.addStringParameter(c.command, "Argument for this command", c.args.isEmpty() ? "" : c.args[0]);
		else
		{
			for (int i = 0; i < c.args.size(); i++)
			{
				launchArguments.addStringParameter(c.command + " " + String(i + 1), "Argument #" + String(i + 1) + " for this command", c.args[i]);
			}
		}

	}

	for (auto& c : launchArguments.controllables)
	{
		c->setControllableFeedbackOnly(true);
	}
}


KeyMappingsContainer::KeyMappingsContainer() :
	ControllableContainer("Key Mappings")
{
	editorIsCollapsed = true;
}

KeyMappingsContainer::~KeyMappingsContainer()
{

}

var KeyMappingsContainer::getJSONData(bool includeNonOverriden)
{
	var data = ControllableContainer::getJSONData(includeNonOverriden);
	KeyPressMappingSet* kms = getCommandManager().getKeyMappings();
	std::unique_ptr<XmlElement> xmlElement(kms->createXml(true));
	String xmlData = xmlElement->toString();
	data.getDynamicObject()->setProperty("keyMappings", xmlData);
	return data;
}

void KeyMappingsContainer::loadJSONDataInternal(var data)
{
	ControllableContainer::loadJSONDataInternal(data);
	var keyMappingsData = data.getProperty("keyMappings", "");
}

InspectableEditor* KeyMappingsContainer::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new KeyMappingsContainerEditor(this, isRoot);
}
