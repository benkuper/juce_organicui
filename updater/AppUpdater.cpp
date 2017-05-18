#include "AppUpdater.h"
/*
  ==============================================================================

    AppUpdater.cpp
    Created: 8 Apr 2017 4:26:46pm
    Author:  Ben

  ==============================================================================
*/


juce_ImplementSingleton(AppUpdater)

void AppUpdater::setURLs(URL _updateURL, URL _downloadURL)
{
	updateURL = _updateURL;  
	downloadURL = _downloadURL;  
}

void AppUpdater::checkForUpdates()
{
	
	if (updateURL.isEmpty() || downloadURL.isEmpty()) return;

	StringPairArray responseHeaders;
	int statusCode = 0; 
	ScopedPointer<InputStream> stream(updateURL.createInputStream(false, nullptr, nullptr, String(),
		200, // timeout in millisecs
		&responseHeaders, &statusCode));

	if (statusCode != 200) LOG("Failed to connect, status code = " + String(statusCode)); 
	else if (stream != nullptr)
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
				LOG("App is up to date.");
			}
		} else
		{
			LOG("Error, update file is not valid");
		}
		
	}	
}
