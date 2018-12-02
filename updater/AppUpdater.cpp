#include "AppUpdater.h"
/*
  ==============================================================================

    AppUpdater.cpp
    Created: 8 Apr 2017 4:26:46pm
    Author:  Ben

  ==============================================================================
*/


juce_ImplementSingleton(AppUpdater)

String getAppVersion();

#define FORCE_UPDATE 0 //to test

AppUpdater::AppUpdater() :
	Thread("appUpdater"),
	queuedNotifier(30)
{
	addAsyncUpdateListener(this);
	progression = new FloatParameter("Progression", "The progression of the download", 0, 0, 1);
}

AppUpdater::~AppUpdater()
{
	queuedNotifier.cancelPendingUpdate();
	signalThreadShouldExit();
	waitForThreadToExit(5000);
}

  void AppUpdater::setURLs(URL _updateURL, String _downloadURLBase, String _filePrefix)
{
	updateURL = _updateURL;  
	filePrefix = _filePrefix;
	downloadURLBase = _downloadURLBase;  
	if (!downloadURLBase.endsWithChar('/')) downloadURLBase += "/";
}

  String AppUpdater::getDownloadFileName(String version, bool beta, String extension)
  {
	  String fileURL = filePrefix + "-";
#if JUCE_WINDOWS
	  fileURL += "win-x64";
#elif JUCE_MAC
	  fileURL += "osx";
#elif JUCE_LINUX
	  fileURL += "linux";
#endif

	  fileURL += "-" + version + "." + extension;
	  return fileURL;
  }

  void AppUpdater::checkForUpdates()
	{
		if (updateURL.isEmpty() || downloadURLBase.isEmpty()) return;
		startThread();
	}

void AppUpdater::showDialog(bool beta, String title, String msg, String changelog)
{
	
	progression->setValue(0);

	updateWindow = new UpdateDialogWindow(msg, changelog, progression);
	DialogWindow::LaunchOptions dw;
	dw.content.set(updateWindow, false);
	dw.dialogTitle = title;
	dw.escapeKeyTriggersCloseButton = true;
	dw.dialogBackgroundColour = BG_COLOR;
	dw.launchAsync();
	
}

void AppUpdater::downloadUpdate()
{

	DBG("Download file name " << downloadingFileName);

	targetDir.createDirectory();

#if JUCE_WINDOWS
	File targetFile = File::getSpecialLocation(File::tempDirectory).getChildFile(downloadingFileName);
#else	
	File targetFile = targetDir.getChildFile(downloadingFileName);
	if (targetFile.existsAsFile()) targetFile.deleteFile();
#endif

	downloadingFileName = targetFile.getFileName();

	URL downloadURL = URL(downloadURLBase + downloadingFileName);

	LOG("Downloading " + downloadURL.toString(false) + "...");
	downloadTask = downloadURL.downloadToFile(targetFile, "", this);

	if (downloadTask == nullptr)
	{
		LOGERROR("Error while downloading " + downloadingFileName + ",\ntry downloading it directly from the website.");
		queuedNotifier.addMessage(new AppUpdateEvent(AppUpdateEvent::DOWNLOAD_ERROR));
	}
	queuedNotifier.addMessage(new AppUpdateEvent(AppUpdateEvent::DOWNLOAD_STARTED));
}

void AppUpdater::run()
{
	if (Engine::mainEngine == nullptr) return;

    //First cleanup update_temp directory
    targetDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("update_temp");
    if (targetDir.exists()) targetDir.deleteRecursively();

    
	StringPairArray responseHeaders;
	int statusCode = 0;
	ScopedPointer<InputStream> stream(updateURL.createInputStream(false, nullptr, nullptr, String(),
		2000, // timeout in millisecs
		&responseHeaders, &statusCode));
#if JUCE_WINDOWS
	if (statusCode != 200)
	{
		LOGWARNING("Failed to connect, status code = " + String(statusCode));
		return;
	}
#endif

	DBG("AppUpdater:: Status code " << statusCode);

	if (stream != nullptr)
	{
		String content = stream->readEntireStreamAsString();
		var data = JSON::parse(content);

		if (data.isObject())
		{

#if !JUCE_DEBUG
			if (data.getProperty("testing", false)) return;
#endif

			bool thisIsBeta = Engine::mainEngine->isBetaVersion;

			bool shouldCheckForBeta = true;
			if (!GlobalSettings::getInstance()->checkBetaUpdates->boolValue()) shouldCheckForBeta = false;
			else if (!thisIsBeta && GlobalSettings::getInstance()->onlyCheckBetaFromBeta->boolValue()) shouldCheckForBeta = false;

			var betaData = data.getProperty("betaversion", var());
			var stableData = data.getProperty("stableversion", var());

			String betaVersion = betaData.getProperty("version", "");
			String stableVersion = stableData.getProperty("version", "");

			var dataToCheck;
			bool dataIsBeta = false;
			int dataBetaVersion = 0;
			if (shouldCheckForBeta && Engine::mainEngine->versionIsNewerThan(betaVersion, stableVersion))
			{
				DBG("Beta is newer : " << betaVersion);
				dataIsBeta = true;
				dataToCheck = betaData;
				dataBetaVersion = Engine::mainEngine->getBetaVersion(betaVersion);
			} else
			{
				DBG("Stable is newer or not checking for beta " << stableVersion);
				dataIsBeta = false;
				dataToCheck = stableData;
			} 

#if !FORCE_UPDATE
			if (Engine::mainEngine->checkFileVersion(dataToCheck.getDynamicObject(), true))
			{
#endif
				String version = dataToCheck.getProperty("version", "");
				String msg = "A new " + String(dataIsBeta ? "BETA " : "") + "version of "+ProjectInfo::projectName+" is available : " + version + ", do you want to update the app ?\nYou can also deactivate updates in the preferences.";

				Array<var> * changelog = dataToCheck.getProperty("changelog", var()).getArray();
				String changelogString = "Changes since your version :\n\n";
				changelogString += "Version " + version + ":\n"; 
				for (auto &c : *changelog) changelogString += c.toString() + "\n";
				changelogString += "\n\n";

				Array<var> * oldChangelogs = data.getProperty("archives", var()).getArray();
				for (int i = oldChangelogs->size() - 1; i >= 0; i--)
				{
					var ch = oldChangelogs->getUnchecked(i);
					String chVersion = ch.getProperty("version", "1.0.0");
					if (Engine::mainEngine->versionIsNewerThan(getAppVersion(), chVersion)) break;

					changelogString += "Version " + chVersion + ":\n";
					Array<var> * versionChangelog = ch.getProperty("changelog", var()).getArray();
					for (auto &c : *versionChangelog) changelogString += c.toString() + "\n";
					changelogString += "\n\n"; 
				}


				String title = dataIsBeta ? "New BETA version available" : "New version available";

				String extension = "zip";
				#if JUCE_WINDOWS
								extension = data.getProperty("winExtension", "zip");
				#elif JUCE_MAC
								extension = data.getProperty("osxExtension", "zip");
				#elif JUCE_LINUX
								extension = data.getProperty("linuxExtension", "zip");
				#endif

				downloadingFileName = getDownloadFileName(version, dataIsBeta, extension);

				queuedNotifier.addMessage(new AppUpdateEvent(AppUpdateEvent::UPDATE_AVAILABLE, dataIsBeta, title, msg, changelogString));

#if !FORCE_UPDATE
			} else
			{
				LOG("App is up to date :) (Latest version online : " << dataToCheck.getProperty("version", "").toString() << ")");
			}
#endif
		} else
		{
			LOGERROR("Error while checking updates, update file is not valid");
		}

	} else
	{
		LOGERROR("Error while trying to access to the update file");
	}
}

void AppUpdater::finished(URL::DownloadTask * task, bool success)
{
	if (!success)
	{ 
		LOGERROR("Error while downloading " + downloadingFileName + ",\ntry downloading it directly from the website.\nError code : " + String(task->statusCode()));
		queuedNotifier.addMessage(new AppUpdateEvent(AppUpdateEvent::DOWNLOAD_ERROR)); return;
	}

#if JUCE_WINDOWS
	File f = File::getSpecialLocation(File::tempDirectory).getChildFile(downloadingFileName);
#else
    File appFile = File::getSpecialLocation(File::currentApplicationFile);
    File appDir = appFile.getParentDirectory();
    
	File f = appDir.getChildFile("update_temp/" + downloadingFileName);
#endif
	if (!f.exists())
	{
		DBG("File doesn't exist");
		return;
	}

	if (f.getSize() < 1000000) //if file is less than 1Mo, got a problem
	{
		LOGERROR("Wrong file size, try downloading it directly from the website");
		return;
	}

#if JUCE_WINDOWS


#else
	File td = f.getParentDirectory();
	{
		ZipFile zip(f);
		zip.uncompressTo(td);
		Array<File> filesToCopy;

		appFile.moveFileTo(td.getNonexistentChildFile("oldApp", appFile.getFileExtension()));

		DBG("Move to " << appDir.getFullPathName());
		for (int i = 0; i < zip.getNumEntries(); i++)
		{
			File zf = td.getChildFile(zip.getEntry(i)->filename);
			DBG("File exists : " << (int)f.exists());
			zf.copyFileTo(appDir.getChildFile(zip.getEntry(i)->filename));
			//DBG("Move result for " << zf.getFileName() << " = " << (int)result);
		}
	}
#endif
    

	queuedNotifier.addMessage(new AppUpdateEvent(AppUpdateEvent::UPDATE_FINISHED, f));
}

void AppUpdater::progress(URL::DownloadTask * task, int64 bytesDownloaded, int64 totalLength)
{
	progression->setValue(bytesDownloaded*1.0f/ totalLength);

	int percent = (int)(progression->floatValue() * 100);
	queuedNotifier.addMessage(new AppUpdateEvent(AppUpdateEvent::DOWNLOAD_PROGRESS));
	LOG("Progress : " << percent);
}

void AppUpdater::newMessage(const AppUpdateEvent & e)
{
	switch(e.type)
	{
	case AppUpdateEvent::UPDATE_AVAILABLE:
		showDialog(e.beta, e.title, e.msg,e.changelog);
		break;

	case AppUpdateEvent::DOWNLOAD_ERROR:
	case AppUpdateEvent::UPDATE_FINISHED:
		updateWindow->getTopLevelComponent()->exitModalState(0);
		break;
            
        default:
            break;
	}
}

UpdateDialogWindow::UpdateDialogWindow(const String & msg, const String & changelog, FloatParameter * progression) :
	okButton("OK"),
	cancelButton("Cancel")
{
	addAndMakeVisible(&msgLabel);
	addAndMakeVisible(&changelogLabel);

	msgLabel.setColour(msgLabel.textColourId, TEXT_COLOR);
	msgLabel.setText(msg, dontSendNotification);

	changelogLabel.setMultiLine(true);
	changelogLabel.setColour(changelogLabel.backgroundColourId, BG_COLOR.darker());
	changelogLabel.setColour(changelogLabel.textColourId, TEXT_COLOR);
	changelogLabel.setScrollBarThickness(8);
	changelogLabel.setReadOnly(true);
	changelogLabel.setText(changelog);

	addAndMakeVisible(&okButton);
	addAndMakeVisible(&cancelButton);

	okButton.addListener(this);
	cancelButton.addListener(this);

	progressionUI = progression->createSlider();
	progressionUI->showLabel = false;
	progressionUI->showValue = false;
	addAndMakeVisible(progressionUI);

	setSize(600, 600);
}

void UpdateDialogWindow::resized()
{
	juce::Rectangle<int> r = getLocalBounds().reduced(10);
	juce::Rectangle<int> br = r.removeFromBottom(20);
	r.removeFromBottom(10);

	progressionUI->setBounds(r.removeFromBottom(8));
	r.removeFromBottom(10);


	msgLabel.setBounds(r.removeFromTop(100));
	r.removeFromTop(10);
	changelogLabel.setBounds(r);

	cancelButton.setBounds(br.removeFromRight(100));
	br.removeFromRight(10);
	okButton.setBounds(br.removeFromRight(100));
}

void UpdateDialogWindow::buttonClicked(Button * b)
{
	if (b == &okButton) AppUpdater::getInstance()->downloadUpdate();
	else if (b == &cancelButton)
	{
		getTopLevelComponent()->exitModalState(0);
	}
}
