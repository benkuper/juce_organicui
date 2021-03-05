/*
  ==============================================================================

    DashboardManager.h
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#if ORGANICUI_USE_WEBSERVER
#include <juce_simpleweb/juce_simpleweb.h>
#endif


#if ORGANICUI_USE_SERVUS
#include "servus/servus.h"

class ServusThread :
	public Thread
{
public:
	ServusThread();
	servus::Servus servus;
	void setupZeroconf();
	void run() override;
};
#endif


class DashboardManager :
	public BaseManager<Dashboard>,
	public Dashboard::DashboardListener,
	public EngineListener
#if ORGANICUI_USE_WEBSERVER
	,public SimpleWebSocketServer::RequestHandler
	,public SimpleWebSocketServer::Listener
	,public URL::DownloadTask::Listener
#endif
{
public:
	juce_DeclareSingleton(DashboardManager, true);

	DashboardManager();
	~DashboardManager();

	BoolParameter* editMode;
	BoolParameter* snapping;

	CommentManager commentManager;


	void addItemInternal(Dashboard* item, var data) override;
	void removeItemInternal(Dashboard* item) override;


	void itemDataFeedback(var data) override;

	void endLoadFile() override;

#if ORGANICUI_USE_WEBSERVER
	std::unique_ptr<SimpleWebSocketServer> server;

	URL downloadURL;
	File serverRootPath;
	File downloadedFileZip;
	std::unique_ptr<URL::DownloadTask> downloadTask;

	void setupDownloadURL(const String& downloadURL);

	void downloadDashboardFiles();
	void progress(URL::DownloadTask* task, int64 bytesDownloaded, int64 bytesTotal) override;
	void finished(URL::DownloadTask* task, bool success) override;

	void setupServer();
	void connectionOpened(const String& id) override;
	void messageReceived(const String& id, const String& message) override;
	void connectionClosed(const String& id, int status, const String& reason) override;

	var getServerData();

	// Inherited via RequestHandler
	virtual bool handleHTTPRequest(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) override;

#endif


private:
#if ORGANICUI_USE_SERVUS
	ServusThread servusThread;
#endif


	
};

