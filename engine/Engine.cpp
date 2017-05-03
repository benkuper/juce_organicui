#include "Engine.h"
/*
 ==============================================================================

 Engine.cpp
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */

Engine * Engine::mainEngine = nullptr;

Engine::Engine(const String & fileName, const String & fileExtension, ApplicationProperties * _appProperties, const String &_appVersion) :
	ControllableContainer("Root"),
	FileBasedDocument(fileExtension,
		"*"+fileExtension,
		"Load a "+fileName,
		"Save a "+fileName),
	appProperties(_appProperties),
	appVersion(_appVersion)
{
	skipControllableNameInAddress = true;

	//to move into engine
	Logger::setCurrentLogger(CustomLogger::getInstance());

	addChildControllableContainer(DashboardManager::getInstance());

	InspectableSelectionManager::getInstance(); //selectionManager constructor
	ScriptUtil::getInstance(); //trigger ScriptUtil constructor
}

Engine::~Engine(){

//delete managers

  InspectableSelectionManager::deleteInstance();
 
  DashboardManager::deleteInstance();
  Outliner::deleteInstance();

  PresetManager::deleteInstance();
  CustomLogger::deleteInstance();
  Logger::setCurrentLogger(nullptr);

  ControllableFactory::deleteInstance();

  ScriptUtil::deleteInstance();
  ShapeShifterFactory::deleteInstance();

  //UndoMaster::deleteInstance();
}

void Engine::parseCommandline(const String & commandLine){

  for (auto & c:StringUtil::parseCommandLine(commandLine)){

    if(c.command== "f"|| c.command==""){
      if(c.args.size()==0){
        LOG("no file provided for command : "+c.command);
        jassertfalse;
        continue;
      }
      String fileArg = c.args[0];
      if (File::isAbsolutePath(fileArg)) {
        File f(fileArg);
        if (f.existsAsFile()) loadDocument(f);
      }
      else {
        NLOG("Engine","File : " << fileArg << " not found.");
      }
    }

  }

}

void Engine::clear() {


	if (Outliner::getInstanceWithoutCreating())
	{
		Outliner::getInstanceWithoutCreating()->clear();
		Outliner::getInstanceWithoutCreating()->enabled = false;
	}

	if (InspectableSelectionManager::getInstanceWithoutCreating())
	{
		InspectableSelectionManager::getInstance()->clearSelection();
		InspectableSelectionManager::getInstance()->setEnabled(false);
	}

	DashboardManager::getInstance()->clear();
	PresetManager::getInstance()->clear();
	
	clearInternal();

	if (Outliner::getInstanceWithoutCreating()) Outliner::getInstanceWithoutCreating()->enabled = true;
	if (InspectableSelectionManager::getInstanceWithoutCreating()) InspectableSelectionManager::getInstance()->setEnabled(true);


	changed();    //fileDocument
}
