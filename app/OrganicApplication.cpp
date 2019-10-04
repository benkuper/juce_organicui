
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
ControllableContainer * getAppSettings() { return &(getApp().appSettings); }
ApplicationProperties& getAppProperties() { return *getApp().appProperties; }
OpenGLContext * getOpenGLContext() { return &getApp().mainComponent->openGLContext; }
ApplicationCommandManager& getCommandManager() { return getApp().commandManager; }
OrganicApplication::MainWindow * getMainWindow() { return getApp().mainWindow.get(); }

#define TEST_CRASH 0

OrganicApplication::OrganicApplication(const String &appName, bool useWindow) :
	appSettings("Other Settings"),
	engine(nullptr),
	mainComponent(nullptr),
	useWindow(useWindow)
{
	PropertiesFile::Options options;
	options.applicationName = appName;
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Preferences";
	appProperties = std::make_unique<ApplicationProperties>();
	appProperties->setStorageParameters(options);
}

const String OrganicApplication::getApplicationName() { return ProjectInfo::projectName; }
const String OrganicApplication::getApplicationVersion() { return ProjectInfo::versionString; }

void OrganicApplication::initialise(const String & commandLine)
{
	initialiseInternal(commandLine);

	CommandLineElements commands = StringUtil::parseCommandLine(commandLine);
	for (auto& c : commands)
	{
		if (c.command == "r") getAppProperties().getUserSettings()->getFile().deleteFile();
	}

	GlobalSettings::getInstance()->addChildControllableContainer(&appSettings,false, GlobalSettings::getInstance()->controllableContainers.size()-1);
	
	var gs = JSON::fromString(getAppProperties().getUserSettings()->getValue("globalSettings", ""));
	GlobalSettings::getInstance()->loadJSONData(gs);

	jassert(engine != nullptr);
	if (mainComponent == nullptr) mainComponent = std::make_unique<OrganicMainContentComponent>();

	engine->addAsyncEngineListener(this);

	WarningReporter::getInstance(); //force creation after engine creation

	GlobalSettings::getInstance()->selectionManager = InspectableSelectionManager::mainSelectionManager;

	if (useWindow)
	{
		mainWindow.reset(new MainWindow(getApplicationName(), mainComponent.get()));
		updateAppTitle();
	}

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
//#if JUCE_WINDOWS
	CrashDumpUploader::getInstance()->uploadEnabled = GlobalSettings::getInstance()->enableCrashUpload->boolValue();
	CrashDumpUploader::getInstance()->init();
//#endif

#if TIMELINE_ADD_MENU_ITEMS
	TimelineAppCommands::init();
#endif

	afterInit();

#if TEST_CRASH
	Engine* n = nullptr;
	n->getJSONData();
#endif
}

void OrganicApplication::shutdown()
{   
	if (useWindow)
	{
		var boundsVar = var(new DynamicObject());
		juce::Rectangle<int> r = mainWindow->getScreenBounds();

		getAppProperties().getUserSettings()->setValue("windowX", r.getPosition().x);
		getAppProperties().getUserSettings()->setValue("windowY", r.getPosition().y);
		getAppProperties().getUserSettings()->setValue("windowWidth", r.getWidth());
		getAppProperties().getUserSettings()->setValue("windowHeight", r.getHeight());
		getAppProperties().getUserSettings()->setValue("fullscreen", mainWindow->isFullScreen());
		getAppProperties().getUserSettings()->setValue("lastVersion", getApplicationVersion());
	}

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
        if(e.file.getFileExtension() == "zip")
        {
            File appFile = File::getSpecialLocation(File::currentApplicationFile);
            File appDir = appFile.getParentDirectory();
            File tempDir = appDir.getChildFile("temp");
            tempDir.deleteRecursively();
            
            #if JUCE_MAC
                chmod (File::getSpecialLocation(File::currentExecutableFile).getFullPathName().toUTF8(), S_IRWXO | S_IRWXU | S_IRWXG);
            #endif
        }else
        {
            File appFile = File::getSpecialLocation(File::tempDirectory).getChildFile(getApplicationName()+String("_install"+e.file.getFileExtension()));
            e.file.copyFileTo(appFile);
            appFile.startAsProcess();
            
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    }
    break;
    
    default: break;
	}
}

void OrganicApplication::updateAppTitle()
{
	if(useWindow && mainWindow != nullptr) mainWindow->setName(getApplicationName() + " " + getApplicationVersion() + " - " + Engine::mainEngine->getDocumentTitle()+(Engine::mainEngine->hasChangedSinceSaved()?" *":"")); 
}

inline OrganicApplication::MainWindow::MainWindow(String name, OrganicMainContentComponent * mainComponent) :
	DocumentWindow(name,
	Colours::lightgrey,
	DocumentWindow::allButtons),
	mainComponent(mainComponent)
{
	setResizable(true, true);
	setUsingNativeTitleBar(true);
	setContentNonOwned(mainComponent, true);
	setOpaque(true);

	int tx = getAppProperties().getUserSettings()->getIntValue("windowX");
	int ty = getAppProperties().getUserSettings()->getIntValue("windowY");
	int tw = getAppProperties().getUserSettings()->getIntValue("windowWidth");
	int th = getAppProperties().getUserSettings()->getIntValue("windowHeight");

#if JUCE_WINDOWS
	bool fullScreenIsDefault = true;
#else
	bool fullScreenIsDefault = false;
#endif

	bool fs = getAppProperties().getUserSettings()->getBoolValue("fullscreen", fullScreenIsDefault);
	
	setBounds(jmax<int>(tx, 100), jmax<int>(ty, 100), jmax<int>(tw, 1200), jmax<int>(th, 800));

#if !JUCE_LINUX
	setFullScreen(fs);
#endif

#if ! JUCE_MAC
	setMenuBar(mainComponent);
#endif

	setVisible(true);

	mainComponent->init();
	
	if (GlobalSettings::getInstance()->launchMinimised->boolValue()) setMinimised(true);
}

void OrganicApplication::MainWindow::closeButtonPressed() 
{
	JUCEApplication::getInstance()->systemRequestedQuit();
}

void OrganicApplication::MainWindow::visibilityChanged()
{
	if(isShowing()) grabKeyboardFocus();
}
