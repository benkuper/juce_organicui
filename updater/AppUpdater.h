/*
  ==============================================================================

    AppUpdater.h
    Created: 8 Apr 2017 4:26:46pm
    Author:  Ben

  ==============================================================================
*/

#ifndef APPUPDATER_H_INCLUDED
#define APPUPDATER_H_INCLUDED


class AppUpdater :
	public Thread
{
public:
	juce_DeclareSingleton(AppUpdater, true);

	AppUpdater() : Thread("appUpdater") {}
	~AppUpdater();

	URL updateURL;
	URL downloadURL;

	void setURLs(URL _updateURL, URL _downloadURL);

	void checkForUpdates();

	// Inherited via Thread
	virtual void run() override;
};


#endif  // APPUPDATER_H_INCLUDED
