/*
  ==============================================================================

    AppUpdater.h
    Created: 8 Apr 2017 4:26:46pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class UpdateDialogWindow :
	public juce::Component,
	public juce::Button::Listener
{
public:
	UpdateDialogWindow(const juce::String& msg, const juce::String& version, const juce::String &changelog, FloatParameter * progression);
	~UpdateDialogWindow() {}

	juce::String version;
	juce::Label msgLabel;
	juce::TextEditor changelogLabel;

	juce::TextButton okButton;
	juce::TextButton cancelButton;
	juce::TextButton skipThisVersionButton;

	std::unique_ptr<FloatSliderUI> progressionUI;

	void resized() override;

	void buttonClicked(juce::Button * b) override;
};


class  AppUpdateEvent
{
public:
	enum Type { UPDATE_AVAILABLE, DOWNLOAD_STARTED, DOWNLOAD_PROGRESS, DOWNLOAD_ERROR, UPDATE_FINISHED };

	AppUpdateEvent(Type t, juce::File f = juce::File()) : type(t), file(f) {}
	AppUpdateEvent(Type t, juce::String version, bool beta, juce::String title, juce::String msg, juce::String changelog) : type(t), version(version), beta(beta), title(title), msg(msg), changelog(changelog) {}
	
	Type type;
	juce::File file;

	juce::String version;
	bool beta;
	juce::String title;
	juce::String msg;
	juce::String changelog;
};

typedef QueuedNotifier<AppUpdateEvent>::Listener AppUpdaterAsyncListener;


class AppUpdater :
	public juce::Thread,
	public juce::URL::DownloadTask::Listener,
	public AppUpdaterAsyncListener
{
public:
	juce_DeclareSingleton(AppUpdater, true);

	AppUpdater();
	~AppUpdater();

	juce::String updateURL;
	juce::String downloadURLBase;
	juce::String downloadingFileName;
    juce::String extension;
	juce::String filePrefix;
	juce::File targetDir;
	juce::var updateData;

	std::unique_ptr<UpdateDialogWindow> updateWindow;
	std::unique_ptr<FloatParameter> progression;

	std::unique_ptr<juce::URL::DownloadTask> downloadTask;

	void setURLs(juce::StringRef _updateURL, juce::StringRef _downloadURLBase, juce::StringRef filePrefix);

	juce::String getDownloadFileName(juce::StringRef version, bool beta, juce::StringRef extension);
	void checkForUpdates(bool includeSkippedVersion = false);

	void setSkipThisVersion(juce::String version);

	void showDialog(juce::StringRef version, bool beta, juce::StringRef title, juce::StringRef msg, juce::StringRef changelog);
	void downloadUpdate();


	// Inherited via Thread
	virtual void run() override;

	// Inherited via Listener
	virtual void finished(juce::URL::DownloadTask * task, bool success) override;
	virtual void progress(juce::URL::DownloadTask* task, juce::int64 bytesDownloaded, juce::int64 totalLength) override;

	bool openStreamProgressCallback(int /*bytesSent*/, int /*totalBytes*/);

	
	virtual void newMessage(const AppUpdateEvent &e) override;

	QueuedNotifier<AppUpdateEvent> queuedNotifier;
	void addAsyncUpdateListener(AppUpdaterAsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedUpdateListener(AppUpdaterAsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncUpdateListener(AppUpdaterAsyncListener* listener) { queuedNotifier.removeListener(listener); }
};
