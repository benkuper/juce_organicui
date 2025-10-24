
#include "JuceHeader.h"

#if JUCE_MAC //for chmod
#include <sys/types.h>
#include <sys/stat.h>
#include "OrganicApplication.h"
#endif
//
//#if JUCE_WINDOWS_
//LONG WINAPI createMiniDump(LPEXCEPTION_POINTERS exceptionPointers);
//#endif

String getAppVersion() { return getApp().getApplicationVersion(); }
ControllableContainer* getAppSettings() { return &(getApp().appSettings); }
ApplicationProperties& getAppProperties() { return *getApp().appProperties; }
OpenGLContext* getOpenGLContext() { return getApp().mainComponent->openGLContext.get(); }
ApplicationCommandManager& getCommandManager() { return getApp().commandManager; }
OrganicApplication::MainWindow* getMainWindow() { return getApp().mainWindow.get(); }


OrganicApplication::OrganicApplication(const String& appName, bool useWindow, const Image& trayIcon) :
	appSettings("Other Settings"),
	engine(nullptr),
	mainComponent(nullptr),
	useWindow(useWindow),
	trayIconImage(trayIcon)
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

bool OrganicApplication::moreThanOneInstanceAllowed()
{
	String commandline = getCommandLineParameters();
	bool fromCrash = commandline.contains("-c ");
	if (fromCrash) return true;

	//global settings and engine are not initialized yet here, must check the value from the file itself.
	var gs = JSON::fromString(getAppProperties().getUserSettings()->getValue("globalSettings", ""));
	var params = gs["containers"]["startupAndUpdate"]["parameters"];
	for (int i = 0; i < params.size(); i++)
	{
		DBG(" >  " << params[i]["controlAddress"].toString());
		if (params[i]["controlAddress"] == "/allowMultipleInstances") return (bool)params[i]["value"];
	}

	return false;// GlobalSettings::getInstance()->allowMultipleInstances->boolValue();
}

void OrganicApplication::initialise(const String& commandLine)
{
	initialiseInternal(commandLine);

	CommandLineElements commands = StringUtil::parseCommandLine(commandLine);

	bool forceGL = false;
	bool forceNoGL = false;
	for (auto& c : commands)
	{
		if (c.command == "r") clearGlobalSettings();
		else if (c.command == "c") launchedFromCrash = true;
		if (c.command == "headless") useWindow = false;
		if (c.command == "forceGL") forceGL = true;
		else if (c.command == "forceNoGL") forceNoGL = true;
	}

	if (Desktop::getInstance().isHeadless()) useWindow = false;

	GlobalSettings::getInstance()->addChildControllableContainer(&appSettings, false, GlobalSettings::getInstance()->controllableContainers.size() - 1);

	var gs = JSON::fromString(getAppProperties().getUserSettings()->getValue("globalSettings", ""));
	GlobalSettings::getInstance()->loadJSONData(gs);

	if (forceGL) GlobalSettings::getInstance()->useGLRenderer->setValue(true);
	else if (forceNoGL) GlobalSettings::getInstance()->useGLRenderer->setValue(false);

	GlobalSettings::getInstance()->addLaunchArguments(commandLine, commands);


	engine->addAsyncEngineListener(this);
	WarningReporter::getInstance(); //force creation after engine creation
	GlobalSettings::getInstance()->selectionManager = InspectableSelectionManager::mainSelectionManager;

	afterSettingsLoaded();

	if (useWindow)
	{
		jassert(engine != nullptr);
		if (mainComponent == nullptr) mainComponent = std::make_unique<OrganicMainContentComponent>();

		mainWindow.reset(new MainWindow(getApplicationName(), mainComponent.get(), trayIconImage));
		updateAppTitle();
	}

	AppUpdater::getInstance()->addAsyncUpdateListener(this);

	if (GlobalSettings::getInstance()->checkUpdatesOnStartup->boolValue()
		&& !GlobalSettings::getInstance()->launchMinimised->boolValue()
		&& useWindow
		) //only checking updates if there is a UI and it's not minimised by default
	{
		AppUpdater::getInstance()->checkForUpdates();
	}

	HelpBox::getInstance()->loadHelp();

	afterInit();

	bool fileIsLoaded = engine->parseCommandline(commandLine);

	if (launchedFromCrash)
	{
		engine->setChangedFlag(true);
	}
	else if(!fileIsLoaded)
	{
		if (!engine->getFile().existsAsFile()) {
			if (GlobalSettings::getInstance()->openLastDocumentOnStartup->boolValue())  Engine::mainEngine->loadFrom(Engine::mainEngine->getLastDocumentOpened(), true);
			else if (GlobalSettings::getInstance()->openSpecificFileOnStartup->boolValue() && GlobalSettings::getInstance()->fileToOpenOnStartup->stringValue().isNotEmpty())  Engine::mainEngine->loadFrom(File(GlobalSettings::getInstance()->fileToOpenOnStartup->stringValue()), true);
			else
			{
				engine->createNewGraph();
				engine->setChangedFlag(false);
			}
		}
	}

	mainComponent->afterInit();
}

void OrganicApplication::shutdown()
{
	CrashDumpUploader::deleteInstance();
	AppUpdater::deleteInstance();

	if (mainComponent == nullptr) return;

	saveGlobalSettings();

	// Add your application's shutdown code here..
	mainComponent->clear();
	mainWindow = nullptr; // (deletes our window)

}


//==============================================================================

void OrganicApplication::systemRequestedQuit()
{
	Engine::mainEngine->saveIfNeededAndUserAgreesAsync([](FileBasedDocument::SaveResult result)
		{
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
	);

}

void OrganicApplication::anotherInstanceStarted(const String& commandLine)
{
	engine->parseCommandline(commandLine);

	if (mainWindow->trayIcon != nullptr)
	{
		mainWindow->addToDesktop();
		mainWindow->setVisible(true);
		mainComponent->setupOpenGL();
		mainWindow->setTrayIconVisible(false);
	}
	else if (mainWindow->isMinimised())
	{
		mainWindow->setMinimised(false);
	}

	//hack because using toFront doesn't work (only if we aren't already on top)
	if (!mainWindow->isAlwaysOnTop()) {
		mainWindow->setAlwaysOnTop(true);
		mainWindow->grabKeyboardFocus();
		mainWindow->setAlwaysOnTop(false);
	} else {
		mainWindow->grabKeyboardFocus();
	}
}


void OrganicApplication::newMessage(const Engine::EngineEvent& e)
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

void OrganicApplication::newMessage(const AppUpdateEvent& e)
{
	switch (e.type)
	{
	case AppUpdateEvent::DOWNLOAD_STARTED:
		ShapeShifterManager::getInstance()->showContent("Logger");
		break;

	case AppUpdateEvent::UPDATE_FINISHED:
	{
		if (e.file.getFileExtension() == "zip")
		{
			File appFile = File::getSpecialLocation(File::currentApplicationFile);
			File appDir = appFile.getParentDirectory();
			File tempDir = appDir.getChildFile("temp");
			tempDir.deleteRecursively();

#if JUCE_MAC
			chmod(File::getSpecialLocation(File::currentExecutableFile).getFullPathName().toUTF8(), S_IRWXO | S_IRWXU | S_IRWXG);
#endif
		}
		else if (e.file.hasFileExtension("AppImage"))
		{
#ifdef JUCE_LINUX
			// Query AppImage env variables
			auto appImageEnvVar = juce::SystemStats::getEnvironmentVariable("APPIMAGE", "");
			auto argv0EnvVar = juce::SystemStats::getEnvironmentVariable("ARGV0", "");
			auto owdEnvVar = juce::SystemStats::getEnvironmentVariable("OWD", "");

			if (appImageEnvVar.isEmpty() || argv0EnvVar.isEmpty() || owdEnvVar.isEmpty())
			{
				LOGERROR("It seems we're not running from an AppImage, cannot update.");
				return;
			}

			// Copy the downloaded AppImage next to the current one
			File currentAppImageFile = File(appImageEnvVar);
			File targetAppImageFile = currentAppImageFile.getParentDirectory().getChildFile(e.file.getFileName());
			if (!e.file.copyFileTo(targetAppImageFile))
			{
				LOGERROR("Could not copy the downloaded file to current directory.");
				return;
			}

			if (!targetAppImageFile.setExecutePermission(true))
			{
				LOGERROR("Could not give execution permission to the downloaded file.");
				return;
			}

			// If running from a symlink, point it to the new version
			File currentExecutedFile;
			if (File::isAbsolutePath(argv0EnvVar))
			{
				currentExecutedFile = File(argv0EnvVar);
			}
			else if (File::isAbsolutePath(owdEnvVar + "/" + argv0EnvVar))
			{
				currentExecutedFile = File(owdEnvVar + "/" + argv0EnvVar);
			}
			else
			{
				LOGWARNING("Could not determine absolute path to executed file");
				return;
			}

			if (currentExecutedFile.isSymbolicLink())
			{
				if (!targetAppImageFile.createSymbolicLink(currentExecutedFile, true))
				{
					LOGERROR("Could not replace symbolic link: " + currentExecutedFile.getFullPathName());
					return;
				}
			}

			JUCEApplication::getInstance()->systemRequestedQuit();
#else
			LOGERROR("Downloaded file is an AppImage but we're not running on linux");
#endif
		}
		else
		{
			File appFile = File::getSpecialLocation(File::tempDirectory).getChildFile(getApplicationName() + String("_install" + e.file.getFileExtension()));
			e.file.copyFileTo(appFile);
			appFile.startAsProcess();

			JUCEApplication::getInstance()->systemRequestedQuit();
		}
	}
	break;

	default: break;
}
}

void OrganicApplication::clearGlobalSettings()
{

	AlertWindow::showAsync(
		MessageBoxOptions().withIconType(AlertWindow::QuestionIcon)
		.withTitle("So you want a fresh start")
		.withMessage("Are you sure you want to delete the Preferences ? If so, you should definitely restart " + getApplicationName() + " after clearing in order to see changes.")
		.withButton("Yes")
		.withButton("No"),
		[](int result)
		{
			if (result)
			{
				getAppProperties().getUserSettings()->getFile().deleteFile();
				getAppProperties().getUserSettings()->clear();
				LOG("Preferences have been cleared.");
			}
		});

}

void OrganicApplication::saveGlobalSettings()
{

	if (useWindow && mainWindow != nullptr)
	{
		MessageManagerLock mmLock;

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
}

void OrganicApplication::updateAppTitle()
{
	if (useWindow && mainWindow != nullptr && Engine::mainEngine != nullptr) mainWindow->setName(getApplicationName() + " " + getApplicationVersion() + " - " + Engine::mainEngine->getDocumentTitle() + (Engine::mainEngine->hasChangedSinceSaved() ? " *" : ""));
}

inline OrganicApplication::MainWindow::MainWindow(String name, OrganicMainContentComponent* mainComponent, const Image& image) :
	DocumentWindow(name,
		Colours::lightgrey,
		DocumentWindow::allButtons),
	mainComponent(mainComponent),
	iconImage(image)
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




	if (GlobalSettings::getInstance()->launchMinimised->boolValue())
	{
		if (GlobalSettings::getInstance()->closeToSystemTray->boolValue())
		{
			removeFromDesktop();
			setTrayIconVisible(true);
		}
		else
		{
			setMinimised(true);
			setVisible(true);
		}
	}
	else
	{
		setVisible(true);
	}
	
	if (GlobalSettings::getInstance()->alwaysOnTop->boolValue()) {
		setAlwaysOnTop(true);
	}

	mainComponent->init();

}

void OrganicApplication::MainWindow::closeToTray()
{
	setTrayIconVisible(true);
	removeFromDesktop();
	mainComponent->clear();
}

void OrganicApplication::MainWindow::openFromTray()
{
	addToDesktop();
	setVisible(true);
	mainComponent->setupOpenGL();
	setTrayIconVisible(false);
}

void OrganicApplication::MainWindow::closeButtonPressed()
{
	if (GlobalSettings::getInstance()->closeToSystemTray->boolValue())
	{
		closeToTray();
	}
	else
	{
		JUCEApplication::getInstance()->systemRequestedQuit();
	}
}

void OrganicApplication::MainWindow::trayIconMouseDown(const MouseEvent& e)
{
	if (e.mods.isRightButtonDown()) showTrayMenu();
	else
	{
		openFromTray();
	}
}


void OrganicApplication::MainWindow::setTrayIconVisible(bool visible)
{
	if (visible)
	{
		trayIcon.reset(new TrayIcon(iconImage));
		trayIcon->addTrayIconListener(this);
	}
	else
	{
		if (trayIcon) trayIcon->removeFromDesktop();
	}
}

void OrganicApplication::MainWindow::showTrayMenu()
{
	PopupMenu p;
	addTrayMenuOptions(p);
	p.addSeparator();
	p.addItem(-1, "Exit");

	p.showMenuAsync(PopupMenu::Options(), [this](int result)
		{
			if (!result) return;
			if (result == -1) OrganicApplication::quit();
			else this->handlTrayMenuResult(result);
		}
	);

}


void OrganicApplication::MainWindow::visibilityChanged()
{
	if (isShowing())
	{
		grabKeyboardFocus();
	}
}



OrganicApplication::TrayIcon::TrayIcon(const Image& iconImage) :
	SystemTrayIconComponent()
{
	setIconImage(iconImage, iconImage);
}

OrganicApplication::TrayIcon::~TrayIcon()
{
	removeFromDesktop();
}

void OrganicApplication::TrayIcon::mouseDown(const MouseEvent& e)
{
	trayIconListeners.call(&TrayIconListener::trayIconMouseDown, e);
}
