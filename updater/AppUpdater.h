/*
  ==============================================================================

    AppUpdater.h
    Created: 8 Apr 2017 4:26:46pm
    Author:  Ben

  ==============================================================================
*/

#ifndef APPUPDATER_H_INCLUDED
#define APPUPDATER_H_INCLUDED


class UpdateDialogWindow :
	public Component,
	public Button::Listener
{
public:
	UpdateDialogWindow(const String &msg, const String &changelog, FloatParameter * progression);
	~UpdateDialogWindow() {}
	Label msgLabel;
	TextEditor changelogLabel;

	TextButton okButton;
	TextButton cancelButton;

	ScopedPointer<FloatSliderUI> progressionUI;

	void resized() override;

	void buttonClicked(Button * b) override;
};


class  AppUpdateEvent
{
public:
	enum Type { UPDATE_AVAILABLE, DOWNLOAD_STARTED, DOWNLOAD_PROGRESS, DOWNLOAD_ERROR, UPDATE_FINISHED };

	AppUpdateEvent(Type t, File f = File()) : type(t), file(f) {}
	AppUpdateEvent(Type t, bool beta, String title, String msg, String changelog) : type(t), beta(beta), title(title), msg(msg), changelog(changelog) {}
	Type type;
	File file;

	bool beta;
	String title;
	String msg;
	String changelog;
};

typedef QueuedNotifier<AppUpdateEvent>::Listener AppUpdaterAsyncListener;


class AppUpdater :
	public Thread,
	public URL::DownloadTask::Listener,
	public AppUpdaterAsyncListener
{
public:
	juce_DeclareSingleton(AppUpdater, true);

	AppUpdater();
	~AppUpdater();

	URL updateURL;
	String downloadURLBase;
	String downloadingFileName;
	String filePrefix;
	File targetDir;

	ScopedPointer<UpdateDialogWindow> updateWindow;
	ScopedPointer<FloatParameter> progression;

	ScopedPointer<URL::DownloadTask> downloadTask;

	void setURLs(URL _updateURL, String _downloadURLBase, String filePrefix);

	String getDownloadFileName(String version, bool beta, String extension); 
	void checkForUpdates();

	void showDialog(bool beta, String title, String msg, String changelog);
	void downloadUpdate();


	// Inherited via Thread
	virtual void run() override;

	// Inherited via Listener
	virtual void finished(URL::DownloadTask * task, bool success) override;
	virtual void progress(URL::DownloadTask* task, int64 bytesDownloaded, int64 totalLength) override;
	
	virtual void newMessage(const AppUpdateEvent &e) override;

	QueuedNotifier<AppUpdateEvent> queuedNotifier;
	void addAsyncUpdateListener(AppUpdaterAsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedUpdateListener(AppUpdaterAsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncUpdateListener(AppUpdaterAsyncListener* listener) { queuedNotifier.removeListener(listener); }
};


#endif  // APPUPDATER_H_INCLUDED
