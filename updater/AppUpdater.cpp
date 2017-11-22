#include "AppUpdater.h"
/*
  ==============================================================================

    AppUpdater.cpp
    Created: 8 Apr 2017 4:26:46pm
    Author:  Ben

  ==============================================================================
*/


juce_ImplementSingleton(AppUpdater)

AppUpdater::~AppUpdater()
{
	signalThreadShouldExit();
	waitForThreadToExit(1000);
}

  void AppUpdater::setURLs(URL _updateURL, URL _downloadURL)
{
	updateURL = _updateURL;  
	downloadURL = _downloadURL;  
}

void AppUpdater::checkForUpdates()
{
	if (updateURL.isEmpty() || downloadURL.isEmpty()) return;
	startThread();
}

void AppUpdater::run()
{
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
			if (Engine::mainEngine->checkFileVersion(data.getDynamicObject(), true))
			{
				String version = data.getProperty("version", "");
				bool beta = data.getProperty("beta", false);
				Array<var> * changelog = data.getProperty("changelog", var()).getArray();

				String msg = "A new " + String(beta ? "beta " : "") + "version of Chataigne is available : " + version + "\n\nChangelog :\n";

				for (auto &c : *changelog) msg += c.toString() + "\n";
				msg += "\nDo you want to go to the download page ?";

				int result = AlertWindow::showOkCancelBox(AlertWindow::InfoIcon, "New version available", msg, "Yes", "No");
				if (result) downloadURL.launchInDefaultBrowser();
			} else
			{
				LOG("App is up to date :) (Latest version online : " << data.getProperty("version","").toString() << ")");
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
