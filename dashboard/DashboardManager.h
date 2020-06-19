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
#endif

class DashboardManager :
	public BaseManager<Dashboard>
#if ORGANICUI_USE_WEBSERVER
	,public SimpleWebSocket::Listener,
	public Dashboard::DashboardListener
#endif

#if ORGANICUI_USE_SERVUS
	,public Thread
#endif
{
public:
	juce_DeclareSingleton(DashboardManager, true);

	DashboardManager();
	~DashboardManager();

	BoolParameter* editMode;
	BoolParameter* snapping;

#if ORGANICUI_USE_WEBSERVER
	std::unique_ptr<SimpleWebSocket> server;
	
	BoolParameter* enableServer;
	IntParameter* serverPort;


	void setupServer();
	void connectionOpened(const String& id) override;
	void messageReceived(const String& id, const String& message) override;
	void connectionClosed(const String& id, int status, const String& reason) override;
#endif

	void addItemInternal(Dashboard* item, var data) override;
	void removeItemInternal(Dashboard* item) override;

	void itemDataFeedback(var data) override;

	void onContainerParameterChanged(Parameter* p) override;
	void afterLoadJSONDataInternal() override;

#if ORGANICUI_USE_SERVUS
	servus::Servus servus;
	void setupZeroconf();
	void run() override;
#endif
};
