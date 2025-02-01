
/*
 ==============================================================================

 Engine.cpp
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */

#include "JuceHeader.h"

using namespace juce;

Engine* Engine::mainEngine = nullptr;

static OrganicApplication& getApp();
String getAppVersion();
ApplicationProperties& getAppProperties();

Engine::Engine(const String& fileName, const String& fileExtension) :
	ControllableContainer("Root"),
	FileBasedDocument(fileExtension,
		"*" + fileExtension,
		"Load a " + fileName,
		"Save a " + fileName),
	DashboardItemProvider("Generic"),
	fileName(fileName),
	fileExtension(fileExtension),
	lastFileAbsolutePath(""),
	autoSaveIndex(0),
	engineNotifier(10),
	isLoadingFile(false),
	isClearing(false)
{
	//skipControllableNameInAddress = true;
	isBetaVersion = getAppVersion().endsWith("b");
	betaVersion = getBetaVersion(getAppVersion());

	selectionManager.reset(new InspectableSelectionManager(true)); //selectionManager constructor

	Logger::setCurrentLogger(CustomLogger::getInstance());
	addChildControllableContainer(ProjectSettings::getInstance());

	addChildControllableContainer(DashboardManager::getInstance());
	addChildControllableContainer(ParrotManager::getInstance());
	DashboardItemFactory::getInstance()->providers.add(this);

	Detective::getInstance();

	ScriptUtil::getInstance(); //trigger ScriptUtil constructor

	startTimer(1, 60000); //auto-save every 5 minutes
}

Engine::~Engine() {

	//delete managers
	clear();

	isClearing = true;

	controllableContainerListeners.clear();
	engineListeners.clear();

	selectionManager = nullptr; //delete selection manager

	Parameter::ValueInterpolator::Manager::deleteInstance();
	FileCheckTimer::deleteInstance();

	Detective::deleteInstance();
	ParrotManager::deleteInstance();

	DashboardManager::deleteInstance();
	Outliner::deleteInstance();

	//PresetManager::deleteInstance();
	Logger::setCurrentLogger(nullptr);
	CustomLogger::deleteInstance();

	ControllableFactory::deleteInstance();
	ScriptUtil::deleteInstance();
	ShapeShifterFactory::deleteInstance();
	HelpBox::deleteInstance();
	UndoMaster::deleteInstance();
	GlobalSettings::deleteInstance();
	AssetManager::deleteInstance();
	ProjectSettings::deleteInstance();

	WarningReporter::deleteInstance();

	OSCRemoteControl::deleteInstance();

	Engine::mainEngine = nullptr;
}

bool Engine::parseCommandline(const String& commandLine) {

	bool fileIsLoaded = false;
	for (auto& c : StringUtil::parseCommandLine(commandLine)) {

		if (c.command == "f" || c.command == "" || c.command == "c") {
			if (c.args.size() == 0) {
				LOG("no file provided for command : " + c.command);
				jassertfalse;
				continue;
			}

			bool fromCrash = c.command == "c";

			String fileArg = c.args[0];
			if (File::isAbsolutePath(fileArg)) {
				File f(fileArg);
				if (f.existsAsFile())
				{
					if (fromCrash)
					{
						setFile(f);
						Timer::callAfterDelay(100, []() { //wait a bit to be sure the crashed instance is closed
							Engine* e = Engine::mainEngine;
							File f = e->getFile();
							e->loadDocument(f);
							e->setFile(File()); //from crash : force other file to force saving again, but also keep working directory
							f.setAsCurrentWorkingDirectory();
							});
					}
					else
					{
						loadDocument(f);
					}

					fileIsLoaded = true;
				}
			}
			else {
				NLOG("Engine", "File : " << fileArg << " not found.");
			}
		}
	}
	return fileIsLoaded;
}


void Engine::childStructureChanged(ControllableContainer* cc)
{
	ControllableContainer::childStructureChanged(cc);

	if (isLoadingFile || isClearing) return;
	scriptObjectIsDirty = true;
}

PopupMenu Engine::getDashboardCreateMenu(int idOffset)
{
	return ControllableChooserPopupMenu(this, idOffset);
}

DashboardItem* Engine::getDashboardItemFromMenuResult(int result)
{
	ControllableChooserPopupMenu chooser(this, 0);
	Controllable* c = chooser.getControllableForResult(result);
	if (c == nullptr) return nullptr;
	return c->createDashboardItem();
}

void Engine::clear() {

	UndoMaster::getInstance()->clearUndoHistory();

	ParrotManager::getInstance()->clear();

	isClearing = true;
	if (Outliner::getInstanceWithoutCreating())
	{
		Outliner::getInstanceWithoutCreating()->clear();
		Outliner::getInstanceWithoutCreating()->enabled = false;
	}

	if (InspectableSelectionManager::mainSelectionManager != nullptr)
	{
		InspectableSelectionManager::mainSelectionManager->clearSelection();
		InspectableSelectionManager::mainSelectionManager->setEnabled(false);
	}

	ProjectSettings::getInstance()->reset();

	DashboardManager::getInstance()->clear();
	//PresetManager::getInstance()->clear();

	clearInternal();

	WarningReporter::getInstance()->clear();

	if (Outliner::getInstanceWithoutCreating()) Outliner::getInstanceWithoutCreating()->enabled = true;
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->setEnabled(true);

	isClearing = false;

	lastFileAbsolutePath = "";

	changed();    //fileDocument	
	engineListeners.call(&EngineListener::engineCleared);
	engineNotifier.addMessage(new EngineEvent(EngineEvent::ENGINE_CLEARED, this));



}


void Engine::timerCallback(int timerID)
{
	if (isClearing || isLoadingFile) return;

	switch (timerID)
	{
	case 1:
	{
		if (GlobalSettings::getInstance()->enableAutoSave->boolValue())
		{
			if (GlobalSettings::getInstance()->autoSaveCurrentFile->boolValue()) if (getFile().existsAsFile()) saveDocument(getFile());
			saveBackupDocument(autoSaveIndex);
			autoSaveIndex = (autoSaveIndex + 1) % GlobalSettings::getInstance()->autoSaveCount->intValue();
			startTimer(1, 60000 * GlobalSettings::getInstance()->autoSaveTime->intValue());
		}
	}
	break;
	}
}

//Engine Listener here

EngineListener::~EngineListener()
{
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}