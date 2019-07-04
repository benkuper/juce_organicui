#include "Engine.h"

/*
 ==============================================================================

 Engine.cpp
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */


Engine * Engine::mainEngine = nullptr;

static OrganicApplication& getApp();
String getAppVersion();
ApplicationProperties& getAppProperties();

Engine::Engine(const String & fileName, const String & fileExtension) :
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

	DashboardItemFactory::getInstance()->providers.add(this);

	addChildControllableContainer(DashboardManager::getInstance());
	addChildControllableContainer(ProjectSettings::getInstance());
	ScriptUtil::getInstance(); //trigger ScriptUtil constructor

	WarningReporter::getInstance(); //trigger constructor

	startTimer(60000*5); //auto-save every 5 minutes
}

Engine::~Engine() {

	//delete managers
	clear();

	isClearing = true;

	controllableContainerListeners.clear();
	engineListeners.clear();

	selectionManager = nullptr; //delete selection manager

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
	OSCRemoteControl::deleteInstance();
	AssetManager::deleteInstance();
	ProjectSettings::deleteInstance();

	WarningReporter::deleteInstance();

	Engine::mainEngine = nullptr;
}

void Engine::parseCommandline(const String & commandLine) {

	for (auto & c : StringUtil::parseCommandLine(commandLine)) {

		if (c.command == "f" || c.command == "") {
			if (c.args.size() == 0) {
				LOG("no file provided for command : " + c.command);
				jassertfalse;
				continue;
			}
			String fileArg = c.args[0];
			if (File::isAbsolutePath(fileArg)) {
				File f(fileArg);
				if (f.existsAsFile()) loadDocument(f);
			} else {
				NLOG("Engine", "File : " << fileArg << " not found.");
			}
		}
	}
}


void Engine::childStructureChanged(ControllableContainer * cc)
{
	ControllableContainer::childStructureChanged(cc);

	if (isLoadingFile || isClearing) return;
	updateLiveScriptObject();
}

PopupMenu Engine::getDashboardCreateMenu(int idOffset)
{
	return ControllableChooserPopupMenu(this, true, true, idOffset);
}

DashboardItem * Engine::getDashboardItemFromMenuResult(int result)
{
	ControllableChooserPopupMenu chooser(this, true, true, 0);
	Controllable * c = chooser.getControllableForResult(result);
	if (c == nullptr) return nullptr;
	return new DashboardTargetItem(c);
}

void Engine::clear() {

	UndoMaster::getInstance()->clearUndoHistory();

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

	DashboardManager::getInstance()->clear();
	//PresetManager::getInstance()->clear();

	clearInternal();

	if (Outliner::getInstanceWithoutCreating()) Outliner::getInstanceWithoutCreating()->enabled = true;
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->setEnabled(true);

	isClearing = false;

	lastFileAbsolutePath = "";

	changed();    //fileDocument	
	engineListeners.call(&EngineListener::engineCleared);
	engineNotifier.addMessage(new EngineEvent(EngineEvent::ENGINE_CLEARED, this));


}


void Engine::timerCallback()
{
	if (isClearing || isLoadingFile) return;

	if (GlobalSettings::getInstance()->enableAutoSave->boolValue())
	{
		saveBackupDocument(autoSaveIndex);
		autoSaveIndex = (autoSaveIndex + 1) % GlobalSettings::getInstance()->autoSaveCount->intValue();
	}
}

//Engine Listener here

EngineListener::~EngineListener()
{
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}