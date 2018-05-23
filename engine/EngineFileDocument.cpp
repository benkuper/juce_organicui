/*
 ==============================================================================

 MaincComponentFileDocument.cpp
 Created: 25 Mar 2016 7:07:20pm
 Author:  Martin Hermant

 ==============================================================================
 */



 /*================================
  this file implements all methods that are related to saving/loading : basicly iherited from FileBasedDocument
  */

#include "JuceHeader.h" //for project infos
#include "Engine.h"

String Engine::getDocumentTitle() {
	if (!getFile().exists())
		return "New unsaved session";

	return getFile().getFileName();
}

void Engine::changed()
{
	FileBasedDocument::changed();
	engineListeners.call(&EngineListener::fileChanged);
	engineNotifier.addMessage(new EngineEvent(EngineEvent::FILE_CHANGED, this));
}

void Engine::createNewGraph() {
	engineListeners.call(&EngineListener::startLoadFile);
	engineNotifier.addMessage(new EngineEvent(EngineEvent::START_LOAD_FILE, this));
	clear();
	isLoadingFile = true;

	//init with default data here

	setFile(File());
	isLoadingFile = false;
	setChangedFlag(false);
	handleAsyncUpdate();

}

Result Engine::loadDocument(const File& file) {

	if (isLoadingFile) {
		// TODO handle quick reloading of file
		return Result::fail("engine already loading");
	}

	isLoadingFile = true;
	engineListeners.call(&EngineListener::startLoadFile);
	engineNotifier.addMessage(new EngineEvent(EngineEvent::START_LOAD_FILE, this));

	if (InspectableSelectionManager::mainSelectionManager != nullptr)  InspectableSelectionManager::mainSelectionManager->setEnabled(false); //avoid creation of inspector editor while recreating all nodes, controllers, rules,etc. from file

#ifdef MULTITHREADED_LOADING
	fileLoader = new FileLoader(this, file);
	fileLoader->startThread(10);
#else
	loadDocumentAsync(file);
	updateLiveScriptObject();
	triggerAsyncUpdate();
#endif

	lastFileAbsolutePath = getFile().getFullPathName();
	return Result::ok();
}

//Called from fileLoader
void Engine::loadDocumentAsync(const File & file) {

	clearTasks();
	taskName = "Loading File";

	ProgressTask * clearTask = addTask("clearing");
	ProgressTask * parseTask = addTask("parsing");
	ProgressTask * loadTask = addTask("loading");

	clearTask->start();
	clear();
	clearTask->end();

	//  {
	//    MessageManagerLock ml;
	//  }
	ScopedPointer<InputStream> is(file.createInputStream());

	loadingStartTime = Time::currentTimeMillis();
	setFile(file);
	file.getParentDirectory().setAsCurrentWorkingDirectory();

	{
		parseTask->start();
		jsonData = JSON::parse(*is);
		parseTask->end();

		loadTask->start();
		loadJSONData(jsonData, loadTask);
		loadTask->end();


	}// deletes data before launching audio, (data not needed after loaded)

	jsonData = var();
	setChangedFlag(false);
}

bool Engine::allLoadingThreadsAreEnded() {
	return true;//NodeManager::getInstance()->getNumJobs()== 0 && (fileLoader && fileLoader->isEnded);
}

void Engine::fileLoaderEnded() {
	if (allLoadingThreadsAreEnded()) {
		triggerAsyncUpdate();
	}
}


void Engine::handleAsyncUpdate()
{
	isLoadingFile = false;
	if (getFile().exists()) {
		setLastDocumentOpened(getFile());
	}

	//  graphPlayer.setProcessor(NodeManager::getInstance()->mainContainer->getAudioGraph());
	//  suspendAudio(false);
	int64 timeForLoading = Time::currentTimeMillis() - loadingStartTime;
	setChangedFlag(false);
	engineListeners.call(&EngineListener::endLoadFile);
	engineNotifier.addMessage(new EngineEvent(EngineEvent::END_LOAD_FILE, this));

	NLOG("Engine", "Session loaded in " << timeForLoading / 1000.0 << "s");
}



Result Engine::saveDocument(const File& file) {

	bool sameFile = lastFileAbsolutePath == file.getFullPathName();
	var data = getJSONData();

	if (file.exists()) file.deleteFile();
	ScopedPointer<OutputStream> os(file.createOutputStream());
	JSON::writeToStream(*os, data);
	os->flush();

	setLastDocumentOpened(file);
	setChangedFlag(false);
	engineListeners.call(&EngineListener::fileSaved, !sameFile);
	engineNotifier.addMessage(new EngineEvent(EngineEvent::FILE_SAVED, this));

	lastFileAbsolutePath = getFile().getFullPathName();

	return Result::ok();
}

Result Engine::saveBackupDocument(int index)
{

	if (!getFile().existsAsFile()) return Result::ok();

	String curFileName = getFile().getFileNameWithoutExtension();
	File autoSaveDir = getFile().getParentDirectory().getChildFile(curFileName + "_autosave");
	autoSaveDir.createDirectory(); 
	File backupFile = autoSaveDir.getChildFile(curFileName + "_autosave_" + String(index) + fileExtension);
	DBG(backupFile.getFullPathName() << " : " << (int)backupFile.exists());
	var data = getJSONData();

	if (backupFile.exists()) backupFile.deleteFile();
	ScopedPointer<OutputStream> os(backupFile.createOutputStream());
	JSON::writeToStream(*os, data);
	os->flush();

	return Result::ok();
}


File Engine::getLastDocumentOpened() {

	if (appProperties == nullptr) return File();
	RecentlyOpenedFilesList recentFiles;
	recentFiles.restoreFromString(appProperties->getUserSettings()
		->getValue(lastFileListKey));

	return recentFiles.getFile(0);
}




void Engine::setLastDocumentOpened(const File& file) {

	if (appProperties == nullptr) return;
	RecentlyOpenedFilesList recentFiles;
	recentFiles.restoreFromString(appProperties->getUserSettings()
		->getValue(lastFileListKey));

	recentFiles.addFile(file);

	appProperties->getUserSettings()->setValue(lastFileListKey, recentFiles.toString());

}

var Engine::getJSONData()
{
	var data(new DynamicObject());
	var metaData(new DynamicObject());

	metaData.getDynamicObject()->setProperty("version", ProjectInfo::versionString);
	metaData.getDynamicObject()->setProperty("versionNumber", ProjectInfo::versionNumber);

	data.getDynamicObject()->setProperty("metaData", metaData);

	var pData = ProjectSettings::getInstance()->getJSONData();
	if (!pData.isVoid() && pData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty("projectSettings", pData);

	var dData = DashboardManager::getInstance()->getJSONData();
	if (!dData.isVoid() && dData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty("dashboardManager", dData);


	return data;
}

/// ===================
// loading

void Engine::loadJSONData(var data, ProgressTask * loadingTask)
{
	clear();

	DynamicObject * dObject = data.getDynamicObject();
	if (dObject == nullptr)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "File format error", "The file you want to open is not a valid noisette.", "Ok, i guess");
		setFile(File());
		return;
	}


	DynamicObject * md = data.getDynamicObject()->getProperty("metaData").getDynamicObject();
	bool versionChecked = checkFileVersion(md);

	if (!versionChecked)
	{
		String versionString = md->hasProperty("version") ? md->getProperty("version").toString() : "?";
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "You're old, dude !", "File version (" + versionString + ") is not supported anymore.\n(Minimum supported version : " + getMinimumRequiredFileVersion() + ")");
		setFile(File());
		return;
	}


	if (convertURL.isNotEmpty())
	{
		bool appVersionIsNewerThanFileVersion = versionIsNewerThan(appVersion, md->getProperty("version"));
		if (appVersionIsNewerThanFileVersion)
		{
			int result = AlertWindow::showYesNoCancelBox(AlertWindow::QuestionIcon, "File compatibility check", "Your file has been save with an older version of Chataigne, some data may be lost if you load it directly. You can choose to update the file online, load it directly or cancel the operation", "Update", "Load directly", "Cancel");
			if (result == 0)
			{
				setFile(File());
				return;
			}

			switch (result)
			{
			case 1: //update
			{
				URL url = URL(convertURL).withPOSTData(data);
				WebInputStream stream(url, true);
				String convertedData = stream.readEntireStreamAsString();
				if (convertedData.isEmpty())
				{
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Update error", "Could not connect to the update server, please make sure you are connected to internet. You can still reload your file and not update it.", "Well, shit happens");
					setFile(File());
					return;
				}

				data = JSON::parse(convertedData);

				DBG("Converted data : " << convertedData);

				if (data.isVoid())
				{
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Update error", "There is an error with the converted file, the data is badly formatted. I mean, real bad. You can still reload your file and not update it.", "Well, shit happens");
					setFile(File());
					return;
				}

				//continue loading with new data
			}
			break;

			case 2: //load directly
					//do nothing
				break;
			}
		}
	}
	


	//if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->setEnabled(false); //avoid creation of inspector editor while recreating all nodes, controllers, rules,etc. from file
	if (Outliner::getInstanceWithoutCreating() != nullptr) Outliner::getInstance()->setEnabled(false);

	DynamicObject * d = data.getDynamicObject();

	ProgressTask * projectTask = loadingTask->addTask("Project Settings");
	ProgressTask * dashboardTask = loadingTask->addTask("Dashboard");

	loadJSONDataInternalEngine(data, loadingTask);

	projectTask->start();
	if (d->hasProperty("projectSettings")) ProjectSettings::getInstance()->loadJSONData(d->getProperty("projectSettings"));
	projectTask->end();

	dashboardTask->start();
	if (d->hasProperty("dashboardManager")) DashboardManager::getInstance()->loadJSONData(d->getProperty("dashboardManager"));
	dashboardTask->end();


	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->setEnabled(true); //Re enable editor
	if (Outliner::getInstanceWithoutCreating() != nullptr) Outliner::getInstance()->setEnabled(true);
}

bool Engine::checkFileVersion(DynamicObject * metaData, bool checkForNewerVersion)
{
	if (!metaData->hasProperty("version")) return false;
	String versionToCheck = checkForNewerVersion ? appVersion : getMinimumRequiredFileVersion();
	//DBG(metaData->getProperty("version").toString() << " / " << versionToCheck);

	String fVersion = metaData->getProperty("version").toString();

	if (versionToCheck == fVersion && !checkForNewerVersion) return true;

	return versionIsNewerThan(fVersion, versionToCheck);
}

bool Engine::versionIsNewerThan(String versionToCheck, String referenceVersion)
{
	bool referenceVersionIsBeta = false;
	if (referenceVersion.endsWith("b"))
	{
		referenceVersion = referenceVersion.substring(0, referenceVersion.length() - 1);
		referenceVersionIsBeta = true;
	}

	bool versionToCheckIsBeta = false;
	if (versionToCheck.endsWith("b"))
	{
		versionToCheck = versionToCheck.substring(0, versionToCheck.length() - 1);
		versionToCheckIsBeta = true;
	}

	StringArray fileVersionSplit;
	fileVersionSplit.addTokens(versionToCheck, juce::StringRef("."), juce::StringRef("\""));

	StringArray minVersionSplit;
	minVersionSplit.addTokens(referenceVersion, juce::StringRef("."), juce::StringRef("\""));

	int maxVersionNumbers = jmax<int>(fileVersionSplit.size(), minVersionSplit.size());
	while (fileVersionSplit.size() < maxVersionNumbers) fileVersionSplit.add("0");
	while (minVersionSplit.size() < maxVersionNumbers) minVersionSplit.add("0");

	for (int i = 0; i < maxVersionNumbers; i++)
	{
		int fV = fileVersionSplit[i].getIntValue();
		int minV = minVersionSplit[i].getIntValue();
		if (fV > minV) return true;
		else if (fV < minV) return false;
	}

	//if equals return false
	return false;
}

String Engine::getMinimumRequiredFileVersion()
{
	return "1.0.0";
}

//#if JUCE_MODAL_LOOPS_PERMITTED
//File Engine::getSuggestedSaveAsFile (const File& defaultFile){
//
//}
//#endif
