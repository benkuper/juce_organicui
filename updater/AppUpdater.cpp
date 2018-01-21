#include "AppUpdater.h"
/*
  ==============================================================================

    AppUpdater.cpp
    Created: 8 Apr 2017 4:26:46pm
    Author:  Ben

  ==============================================================================
*/


juce_ImplementSingleton(AppUpdater)

AppUpdater::AppUpdater() : 
	Thread("appUpdater"), 
	checkForBetas(true),
	queuedNotifier(30)
{}

AppUpdater::~AppUpdater()
{
	queuedNotifier.cancelPendingUpdate();
	signalThreadShouldExit();
	waitForThreadToExit(1000);
}

  void AppUpdater::setURLs(URL _updateURL, String _downloadURLBase, String _filePrefix)
{
	updateURL = _updateURL;  
	filePrefix = _filePrefix;
	downloadURLBase = _downloadURLBase;  
	if (!downloadURLBase.endsWithChar('/')) downloadURLBase += "/";
}

  String AppUpdater::getDownloadFileName(String version, bool beta)
  {
	  String fileURL = filePrefix + "-";
#if JUCE_WINDOWS
	  fileURL += "win-x64";
#elif JUCE_MAC
	  fileURL += "osx";
#elif JUCE_LINUX
	  fileURL += "linux";
#endif

	  fileURL += "-" + version + "-" + (beta ? "beta" : "stable") + ".zip";

	  return fileURL;
  }

  void AppUpdater::checkForUpdates()
{
	if (updateURL.isEmpty() || downloadURLBase.isEmpty()) return;
	startThread();
}

void AppUpdater::run()
{
    //First cleanup update_temp directory
    File targetDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("update_temp");
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
			bool beta = data.getProperty("beta", false);

			if (!beta || checkForBetas)
			{
				if (Engine::mainEngine->checkFileVersion(data.getDynamicObject(), true))
				{
					String version = data.getProperty("version", "");
					Array<var> * changelog = data.getProperty("changelog", var()).getArray();
					String msg = "A new " + String(beta ? "BETA " : "") + "version of Chataigne is available : " + version + "\n\nChangelog :\n";

					for (auto &c : *changelog) msg += c.toString() + "\n";
                    msg += "\nDo you want to update the app ?";

					

					int result = AlertWindow::showOkCancelBox(AlertWindow::InfoIcon, "New version available", msg, "Yes", "No");
					if (result)
					{
						downloadingFileName = getDownloadFileName(version, beta);
						URL downloadURL = URL(downloadURLBase + downloadingFileName);

						
						targetDir.createDirectory();
						
						File targetFile = targetDir.getChildFile(downloadingFileName);
						if (targetFile.existsAsFile()) targetFile.deleteFile();

						downloadingFileName = targetFile.getFileName();

						LOG("Downloading " + downloadURL.toString(false) + "...");


						downloadTask = downloadURL.downloadToFile(targetFile, "", this);
						if (downloadTask == nullptr)
						{
							LOGERROR("Error while downloading " + downloadingFileName + ",\ntry downloading it directly from the website.");
							queuedNotifier.addMessage(new UpdateEvent(UpdateEvent::DOWNLOAD_ERROR));
						}
						queuedNotifier.addMessage(new UpdateEvent(UpdateEvent::DOWNLOAD_STARTED));
					}
				} else
				{
					LOG("App is up to date :) (Latest version online : " << data.getProperty("version", "").toString() << ")");
				}
			}
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
		queuedNotifier.addMessage(new UpdateEvent(UpdateEvent::DOWNLOAD_ERROR)); return;
	}

    File appFile = File::getSpecialLocation(File::currentApplicationFile);
    File appDir = appFile.getParentDirectory();
    
	File f = appDir.getChildFile("update_temp/" + downloadingFileName);
	if (!f.exists())
	{
		DBG("File doesn't exist");
		return;
	}

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
			bool result = zf.copyFileTo(appDir.getChildFile(zip.getEntry(i)->filename));
			DBG("Move result for " << zf.getFileName() << " = " << (int)result);
		}
	}

	queuedNotifier.addMessage(new UpdateEvent(UpdateEvent::UPDATE_FINISHED));
}

void AppUpdater::progress(URL::DownloadTask * task, int64 bytesDownloaded, int64 totalLength)
{
	int percent = (int)(bytesDownloaded*100.0f / totalLength);
	queuedNotifier.addMessage(new UpdateEvent(UpdateEvent::DOWNLOAD_PROGRESS));
	LOG("Progress : " << percent);
}
