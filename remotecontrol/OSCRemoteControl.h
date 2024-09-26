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
	public juce::Thread,
#endif

#if ORGANICUI_USE_WEBSERVER
	public SimpleWebSocketServer::Listener,
	public SimpleWebSocketServer::RequestHandler,
	public CustomLogger::LoggerListener,
	public WarningReporter::AsyncListener,
#endif
	public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
public:
	juce_DeclareSingleton(OSCRemoteControl, true);

	OSCRemoteControl();
	~OSCRemoteControl();

	IntParameter* localPort;
	juce::OSCReceiver receiver;
	bool receiverIsConnected;
	BoolParameter* logIncoming;
	BoolParameter* logOutgoing;
	BoolParameter* sendFeedbackOnListen;
	BoolParameter* enableSendLogFeedback;

	EnablingControllableContainer manualSendCC;
	juce::OSCSender manualSender;
	StringParameter* manualAddress;
	IntParameter* manualPort;

	void setupReceiver();
	void setupManualSender();

#if ORGANICUI_USE_SERVUS
	servus::Servus servus;

#if ORGANICUI_USE_WEBSERVER
	servus::Servus oscQueryServus;
#endif

	void setupZeroconf();
#endif

	void updateEngineListener();

	void processMessage(const juce::OSCMessage& m, const juce::String& sourceId = "");

	void onContainerParameterChanged(Parameter* p) override;

	void oscMessageReceived(const juce::OSCMessage& m) override;
	void oscBundleReceived(const juce::OSCBundle& b) override;

#if ORGANICUI_USE_SERVUS
	void run() override;
#endif

#if ORGANICUI_USE_WEBSERVER
	std::unique_ptr<SimpleWebSocketServer> server;

	juce::HashMap<juce::String, juce::Array<Controllable*>, juce::DefaultHashFunctions, juce::CriticalSection> feedbackMap;
	juce::HashMap<Controllable*, juce::String> noFeedbackMap;

	std::function<void(juce::var& metaData)> fillHostInfoMetaDataFunc;

	void setupServer();
	bool handleHTTPRequest(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) override;


	void connectionOpened(const juce::String& id) override;
	void messageReceived(const juce::String& id, const juce::String& message) override;
	void dataReceived(const juce::String& id, const juce::MemoryBlock& data) override;
	void connectionClosed(const juce::String& id, int status, const juce::String& reason) override;
	void connectionError(const juce::String& id, const juce::String& message) override;

	void sendOSCQueryFeedback(Controllable* c, const juce::String& excludeId = "");
	void sendOSCQueryStateFeedback(Controllable* c, const juce::String& excludeId = "");
	void sendOSCQueryFeedback(const juce::OSCMessage& m, juce::StringArray excludes = juce::StringArray());
	void sendOSCQueryFeedbackTo(const juce::OSCMessage& m, juce::StringArray ids = juce::StringArray());

	void sendPathAddedFeedback(const juce::String& path);
	void sendPathRemovedFeedback(const juce::String& path);
	void sendPathNameChangedFeedback(const juce::String& oldPath, const juce::String& newPath);

	void controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;
	void controllableStateUpdate(ControllableContainer* cc, Controllable* c) override;

	//void newMessage(const ContainerAsyncEvent& e) override;
	void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

	void newMessage(const CustomLogger::LogEvent& e) override;
	void sendLogFeedback(const juce::String& type, const juce::String& source, const juce::String& message);
	void sendPersistentWarningFeedback(juce::WeakReference<WarningTarget> wt, juce::String address = juce::String(), WarningReporter::WarningReporterEvent::Type type = WarningReporter::WarningReporterEvent::WARNING_REGISTERED);

#endif

	void newMessage(const WarningReporter::WarningReporterEvent& e) override;

	void sendAllManualFeedback();
	void sendManualFeedbackForControllable(Controllable* c);


	class RemoteControlListener
	{
	public:
		virtual ~RemoteControlListener() {}
		virtual void processMessage(const juce::OSCMessage& m, const juce::String& clientId) {}
	};

	DECLARE_INSPECTACLE_CRITICAL_LISTENER(RemoteControl, remoteControl)

};

