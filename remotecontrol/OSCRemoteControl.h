/*
  ==============================================================================

    OSCRemoteControl.h
    Created: 23 Apr 2018 5:00:30pm
    Author:  Ben

  ==============================================================================
*/

#pragma once


#if ORGANICUI_USE_SERVUS
#include "servus/servus.h"
#endif

#if ORGANICUI_USE_WEBSERVER
#include <juce_simpleweb/juce_simpleweb.h>
#include "OSCPacketHelper.h"
#endif

class OSCRemoteControl :
	public EnablingControllableContainer,
#if ORGANICUI_USE_SERVUS
	public Thread,
#endif

#if ORGANICUI_USE_WEBSERVER
	public SimpleWebSocketServer::Listener,
	public SimpleWebSocketServer::RequestHandler,
#endif

	public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>

{
public: 
	juce_DeclareSingleton(OSCRemoteControl, true);

	OSCRemoteControl();
	~OSCRemoteControl();

	IntParameter * localPort;
	OSCReceiver receiver;
	BoolParameter* logIncoming;
	BoolParameter* logOutgoing;

	void setupReceiver();

#if ORGANICUI_USE_SERVUS
	servus::Servus servus;

#if ORGANICUI_USE_WEBSERVER
	servus::Servus oscQueryServus;
#endif

	void setupZeroconf();
#endif

	void processMessage(const OSCMessage &m, const String &sourceId = "");

	void onContainerParameterChanged(Parameter * p) override;
	
	void oscMessageReceived(const OSCMessage &m) override;
	void oscBundleReceived(const OSCBundle &b) override;

#if ORGANICUI_USE_SERVUS
	void run() override;
#endif

#if ORGANICUI_USE_WEBSERVER
	std::unique_ptr<SimpleWebSocketServer> server;

	HashMap<String, Array<Controllable*>, DefaultHashFunctions, CriticalSection> feedbackMap;
	HashMap<Controllable*, String> noFeedbackMap;

	void setupServer();
	bool handleHTTPRequest(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) override;
	var getOSCQueryDataForContainer(ControllableContainer* cc);
	var getOSCQueryDataForControllable(Controllable* c);

	void connectionOpened(const String& id) override;
	void messageReceived(const String& id, const String& message) override;
	void dataReceived(const String& id, const MemoryBlock &data) override;
	void connectionClosed(const String& id, int status, const String& reason) override;
	void connectionError(const String& id, const String& message) override;

	void controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;

	void sendOSCQueryFeedback(Controllable* c, const String & excludeId = "");

#endif

	class RemoteControlListener
	{
	public:
        virtual ~RemoteControlListener(){}
		virtual void processMessage(const OSCMessage &m) {}
	};

	ListenerList<RemoteControlListener> remoteControlListeners;
	void addRemoteControlListener(RemoteControlListener* e) { remoteControlListeners.add(e); }
	void removeRemoteControlListener(RemoteControlListener* e) { remoteControlListeners.remove(e); }

};

