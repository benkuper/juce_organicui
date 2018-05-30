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
	public Thread,
	public URL::DownloadTask::Listener
{
public:
	juce_DeclareSingleton(AppUpdater, true);

	AppUpdater();
	~AppUpdater();

	URL updateURL;
	String downloadURLBase;
	String downloadingFileName;
	String filePrefix;

	ScopedPointer<URL::DownloadTask> downloadTask;

	void setURLs(URL _updateURL, String _downloadURLBase, String filePrefix);

	String getDownloadFileName(String version, bool beta, String extension); 
	void checkForUpdates();

	// Inherited via Thread
	virtual void run() override;

	// Inherited via Listener
	virtual void finished(URL::DownloadTask * task, bool success) override;
	virtual void progress(URL::DownloadTask* task, int64 bytesDownloaded, int64 totalLength) override;


	class  UpdateEvent
	{
	public:
		enum Type { DOWNLOAD_STARTED, DOWNLOAD_PROGRESS, DOWNLOAD_ERROR, UPDATE_FINISHED };

		UpdateEvent(Type t, File f = File()) : type(t), file(f) {}
		Type type;
		File file;
	};

	QueuedNotifier<UpdateEvent> queuedNotifier;
	typedef QueuedNotifier<UpdateEvent>::Listener AsyncListener;


	void addAsyncUpdateListener(AsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedUpdateListener(AsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncUpdateListener(AsyncListener* listener) { queuedNotifier.removeListener(listener); }
};


#endif  // APPUPDATER_H_INCLUDED
