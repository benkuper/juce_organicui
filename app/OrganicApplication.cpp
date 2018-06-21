
#if JUCE_MAC //for chmod
#include <sys/types.h>
#include <sys/stat.h>
#endif
//
//#if JUCE_WINDOWS_
//LONG WINAPI createMiniDump(LPEXCEPTION_POINTERS exceptionPointers);
//#endif

static OrganicApplication& getApp() { return *dynamic_cast<OrganicApplication*>(JUCEApplication::getInstance()); }
String getAppVersion() { return getApp().getApplicationVersion(); }
ApplicationProperties& getAppProperties() { return *getApp().appProperties; }
OpenGLContext * getOpenGLContext() { return &getApp().mainComponent->openGLContext; }
ApplicationCommandManager& getCommandManager() { return getApp().commandManager; }
OrganicApplication::MainWindow * getMainWindow() { return getApp().mainWindow; }


OrganicApplication::OrganicApplication(const String &appName) :
	appSettings("Other Settings"),
	engine(nullptr),
	mainComponent(nullptr)
{
	PropertiesFile::Options options;
	options.applicationName = appName;
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Preferences";
	appProperties = new ApplicationProperties();
	appProperties->setStorageParameters(options);
}

const String OrganicApplication::getApplicationName() { return ProjectInfo::projectName; }
const String OrganicApplication::getApplicationVersion() { return ProjectInfo::versionString; }

void OrganicApplication::initialise(const String & commandLine)
{
	initialiseInternal(commandLine);
	
	GlobalSettings::getInstance()->addChildControllableContainer(&appSettings);
	var gs = JSON::fromString(getAppProperties().getUserSettings()->getValue("globalSettings", ""));
	GlobalSettings::getInstance()->loadJSONData(gs);

	jassert(engine != nullptr);
	if (mainComponent == nullptr) mainComponent = new OrganicMainContentComponent();


	engine->addAsyncEngineListener(this);

	GlobalSettings::getInstance()->selectionManager = InspectableSelectionManager::mainSelectionManager;

	mainWindow = new MainWindow(getApplicationName(), mainComponent);
	
	updateAppTitle();

	AppUpdater::getInstance()->addAsyncUpdateListener(this);

	if (GlobalSettings::getInstance()->checkUpdatesOnStartup->boolValue()) AppUpdater::getInstance()->checkForUpdates();
	HelpBox::getInstance()->loadHelp();

	engine->parseCommandline(commandLine);

	if (!engine->getFile().existsAsFile()) {
		if (GlobalSettings::getInstance()->openLastDocumentOnStartup->boolValue())  Engine::mainEngine->loadFrom(Engine::mainEngine->getLastDocumentOpened(), true);
		else if (GlobalSettings::getInstance()->openSpecificFileOnStartup->boolValue() && GlobalSettings::getInstance()->fileToOpenOnStartup->stringValue().isNotEmpty())  Engine::mainEngine->loadFrom(File(GlobalSettings::getInstance()->fileToOpenOnStartup->stringValue()), true);
		else
		{
			engine->createNewGraph();
			engine->setChangedFlag(false);
		}
	}

	//Crash handler
#if JUCE_WINDOWS
	CrashDumpUploader::getInstance()->init();
#endif

	afterInit();
}

void OrganicApplication::shutdown()
{   
	var boundsVar = var(new DynamicObject());
	juce::Rectangle<int> r = mainWindow->getScreenBounds();

	getAppProperties().getCommonSettings(true)->setValue("windowX", r.getPosition().x);
	getAppProperties().getCommonSettings(true)->setValue("windowY", r.getPosition().y);
	getAppProperties().getCommonSettings(true)->setValue("windowWidth", r.getWidth());
	getAppProperties().getCommonSettings(true)->setValue("windowHeight", r.getHeight());
	getAppProperties().getCommonSettings(true)->setValue("fullscreen", mainWindow->isFullScreen());
	getAppProperties().getCommonSettings(true)->setValue("lastVersion", getApplicationVersion());
	getAppProperties().getCommonSettings(true)->saveIfNeeded();

	getAppProperties().getUserSettings()->setValue("globalSettings", JSON::toString(GlobalSettings::getInstance()->getJSONData()));
	getAppProperties().getUserSettings()->saveIfNeeded();

	// Add your application's shutdown code here..
	mainComponent->clear();
	mainWindow = nullptr; // (deletes our window)

	AppUpdater::deleteInstance();
	CrashDumpUploader::deleteInstance();
}


//==============================================================================

void OrganicApplication::systemRequestedQuit()
{
	FileBasedDocument::SaveResult result = Engine::mainEngine->saveIfNeededAndUserAgrees();
	if (result == FileBasedDocument::SaveResult::userCancelledSave) return;
	else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
	{
		LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
		return;
	}
	// This is called when the app is being asked to quit: you can ignore this
	// request and let the app carry on running, or call quit() to allow the app to close.
	quit();
}

inline void OrganicApplication::anotherInstanceStarted(const String & commandLine)
{
	engine->parseCommandline(commandLine);
}



void OrganicApplication::newMessage(const Engine::EngineEvent & e)
{
	switch (e.type)
	{
	case Engine::EngineEvent::END_LOAD_FILE:
	case Engine::EngineEvent::FILE_SAVED:
	case Engine::EngineEvent::ENGINE_CLEARED:
	case Engine::EngineEvent::FILE_CHANGED:
		updateAppTitle();
		break;
	default:
		//
		break;
	}
}

void OrganicApplication::newMessage(const AppUpdateEvent & e)
{
	switch (e.type)
	{
    case AppUpdateEvent::DOWNLOAD_STARTED:
        ShapeShifterManager::getInstance()->showContent("Logger");
        break;
            
	case AppUpdateEvent::UPDATE_FINISHED:
	{

#if JUCE_WINDOWS
		File appFile = File::getSpecialLocation(File::tempDirectory).getChildFile(getApplicationName()+String("_install.exe"));
		e.file.copyFileTo(appFile);
		appFile.startAsProcess();
#else
		File appFile = File::getSpecialLocation(File::currentApplicationFile);
		File appDir = appFile.getParentDirectory();
		File tempDir = appDir.getChildFile("temp");
		tempDir.deleteRecursively();
#if JUCE_MAC
        chmod (File::getSpecialLocation(File::currentExecutableFile).getFullPathName().toUTF8(), S_IRWXO | S_IRWXU | S_IRWXG);
#endif
		 
		JUCEApplication::getInstance()->systemRequestedQuit();
		appFile.startAsProcess();
#endif
	}
#
		break;
        default:
            break;
	}
}

void OrganicApplication::updateAppTitle()
{
	if(mainWindow != nullptr) mainWindow->setName(getApplicationName() + " " + getApplicationVersion() + " - " + Engine::mainEngine->getDocumentTitle()+(Engine::mainEngine->hasChangedSinceSaved()?" *":"")); 
}

inline OrganicApplication::MainWindow::MainWindow(String name, OrganicMainContentComponent * mainComponent) :
	DocumentWindow(name,
	Colours::lightgrey,
	DocumentWindow::allButtons),
	mainComponent(mainComponent)
{
	setResizable(true, true);
	setUsingNativeTitleBar(true);
	setContentOwned(mainComponent, true);
	setOpaque(true);

	int tx = getAppProperties().getCommonSettings(true)->getIntValue("windowX");
	int ty = getAppProperties().getCommonSettings(true)->getIntValue("windowY");
	int tw = getAppProperties().getCommonSettings(true)->getIntValue("windowWidth");
	int th = getAppProperties().getCommonSettings(true)->getIntValue("windowHeight");
	bool fs = getAppProperties().getCommonSettings(true)->getBoolValue("fullscreen", true);
	
	setBounds(jmax<int>(tx, 20), jmax<int>(ty, 20), jmax<int>(tw, 100), jmax<int>(th, 100));
	setFullScreen(fs);

#if ! JUCE_MAC
	setMenuBar(mainComponent);
#endif

	setVisible(true);

	mainComponent->init();
	
}

void OrganicApplication::MainWindow::closeButtonPressed() 
{
	JUCEApplication::getInstance()->systemRequestedQuit();
}

void OrganicApplication::MainWindow::visibilityChanged()
{
	if(isShowing()) grabKeyboardFocus();
}
