/*
  ==============================================================================

	OSCRemoteControl.cpp
	Created: 23 Apr 2018 5:00:30pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"
juce_ImplementSingleton(OSCRemoteControl)

#if ORGANIC_USE_WEBSERVER
#include "OSCPacketHelper.h"
#include "OSCRemoteControl.h"
#endif

#ifndef ORGANIC_REMOTE_CONTROL_PORT
#define ORGANIC_REMOTE_CONTROL_PORT 42000
#endif

OSCRemoteControl::OSCRemoteControl() :
	EnablingControllableContainer("OSC Remote Control")
#if ORGANICUI_USE_SERVUS
	, Thread("Global Zeroconf")
	, servus("_osc._udp")

#if ORGANICUI_USE_WEBSERVER
	, oscQueryServus("_oscjson._tcp")
#endif

#endif
	, manualSendCC("Manual OSC Send")
{

	saveAndLoadRecursiveData = true; //can be useful when app include other settings there

	enabled->setValue(false);

	localPort = addIntParameter("Local Port", "Local port to connect to for global control over the application", ORGANIC_REMOTE_CONTROL_PORT, 1, 65535);
	logIncoming = addBoolParameter("Log Incoming", "If checked, this will log incoming messages", false);
	logOutgoing = addBoolParameter("Log Outgoing", "If checked, this will log outgoing messages", false);

	receiver.addListener(this);
	receiver.registerFormatErrorHandler(&OSCHelpers::logOSCFormatError);

	manualAddress = manualSendCC.addStringParameter("Address", "Address to send to", "127.0.0.1");
	manualPort = manualSendCC.addIntParameter("Port", "Port to send to", ORGANIC_REMOTE_CONTROL_PORT + 1, 1, 65535);

	manualSendCC.enabled->setDefaultValue(false);
	addChildControllableContainer(&manualSendCC);

}

OSCRemoteControl::~OSCRemoteControl()
{
#if ORGANICUI_USE_SERVUS
	stopThread(1000);
#endif

#if ORGANICUI_USE_WEBSERVER
	if (server != nullptr)
	{
		server->stop();
		server.reset();
	}

#endif

}

void OSCRemoteControl::setupReceiver()
{
	//if (receiveCC == nullptr) return;

	receiver.disconnect();
	receiverIsConnected = false;

#if ORGANICUI_USE_WEBSERVER
	updateEngineListener();
#endif

	if (!enabled->boolValue())
	{
#if ORGANICUI_USE_SERVUS
		setupZeroconf();
#endif
		return;
	}

	//if (!receiveCC->enabled->boolValue()) return;
	receiverIsConnected = receiver.connect(localPort->intValue());


	if (receiverIsConnected)
	{

#if ORGANICUI_USE_WEBSERVER
		setupServer();
		updateEngineListener();
#endif

		NLOG(niceName, "Now receiving on port : " + localPort->stringValue());
#if ORGANICUI_USE_SERVUS
		setupZeroconf();
#endif
	}
	else
	{
		NLOGERROR(niceName, "Error binding port " + localPort->stringValue());
	}


	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);

	Array<String> ips;
	for (auto& a : ad) ips.add(a.toString());
	ips.sort();
	String s = "Local IPs:";
	for (auto& ip : ips) s += String("\n > ") + ip;

	NLOG(niceName, s);
}

void OSCRemoteControl::setupManualSender()
{
	manualSender.disconnect();
	updateEngineListener();

	if (!manualSendCC.enabled->boolValue()) return;

	manualSender.connect(manualAddress->stringValue(), manualPort->intValue());
	updateEngineListener();
}

#if ORGANICUI_USE_SERVUS
void OSCRemoteControl::setupZeroconf()
{
	if (Engine::mainEngine->isClearing || localPort == nullptr) return;
	if (!isThreadRunning()) startThread();
}
#endif

void OSCRemoteControl::updateEngineListener()
{
#if ORGANICUI_USE_WEBSERVER
	Engine::mainEngine->removeAsyncContainerListener(this);

	bool shouldListen = false;
	if (!enabled->boolValue()) return;
	if (manualSendCC.enabled->boolValue()) shouldListen = true;
	if (receiverIsConnected) shouldListen = true;

	if (shouldListen) Engine::mainEngine->addAsyncContainerListener(this);
#endif
}


void OSCRemoteControl::processMessage(const OSCMessage& m, const String& sourceId)
{
	String add = m.getAddressPattern().toString();

	if (logIncoming->boolValue())
	{
		String s = add;
		for (auto& a : m)
		{
			s += "\n" + OSCHelpers::getStringArg(a);
		}

		NLOG(niceName, "Received : " << s);
	}



	if (add == "/openFile")
	{

		if (m.size() < 1) LOGWARNING("Cannot open file, no argument provided");
		else
		{
			if (!m[0].isString())
			{
				LOGWARNING("Cannot open file, argument #0 is not a string");
				return;
			}

			File f(m[0].getString());
			if (!f.exists())
			{
				LOGWARNING("File doesn't exist : " << f.getFullPathName());
				return;
			}

			MessageManagerLock mmLock;
			Engine::mainEngine->loadDocument(f);
		}
	}
	else if (add == "/newFile")
	{
		MessageManagerLock mmLock;
		Engine::mainEngine->createNewGraph();
	}
	else if (add == "/saveFile")
	{
		MessageManagerLock mmLock;
		if (m.size() >= 1 && m[0].isString())
		{
			File f = File::getSpecialLocation(File::userDocumentsDirectory).getNonexistentChildFile(getApp().getApplicationName() + "/" + m[0].getString(), Engine::mainEngine->fileExtension, true);

			Engine::mainEngine->saveAsAsync(f, false, false, false, [](int result) {});
		}
		else
		{
			if (Engine::mainEngine->getFile().existsAsFile()) Engine::mainEngine->saveAsync(false, false, nullptr);
			else
			{
				File f = File::getSpecialLocation(File::userDocumentsDirectory).getNonexistentChildFile(getApp().getApplicationName() + "/default", Engine::mainEngine->fileExtension, true);
				Engine::mainEngine->saveAsAsync(f, false, false, false, [](int result) {});
			}
		}
	}
	else if (add == "/closeApp")
	{
		MessageManagerLock mmLock;
		OrganicApplication::quit();
	}
	else if (add == "/toTray")
	{
		MessageManager::callAsync([]() {((OrganicApplication*)OrganicApplication::getInstance())->mainWindow->closeToTray(); });
	}
	else if (add == "/minimize")
	{
		MessageManager::callAsync([]() {((OrganicApplication*)OrganicApplication::getInstance())->mainWindow->setMinimised(true); });
		((OrganicApplication*)OrganicApplication::getInstance())->mainWindow->setMinimised(true);

	}
	else if (add == "/maximize")
	{
		MessageManager::callAsync([]() {
			((OrganicApplication*)OrganicApplication::getInstance())->mainWindow->setMinimised(false);
			((OrganicApplication*)OrganicApplication::getInstance())->mainWindow->openFromTray();
			});
	}
	else if (add == "/syncAll")
	{
		sendAllManualFeedback();
	}
	else
	{

#if ORGANICUI_USE_WEBSERVER
		Controllable* c = OSCHelpers::findControllable(Engine::mainEngine, m);

		if (c != nullptr)
		{
			if (sourceId.isEmpty())
			{
				HashMap<String, Array<Controllable*>, DefaultHashFunctions, CriticalSection>::Iterator it(feedbackMap);
				while (it.next())
				{
					if (it.getKey().contains(m.getSenderIPAddress()))
					{
						noFeedbackMap.set(c, it.getKey());
						break;
					}
				}
			}
			else
			{
				noFeedbackMap.set(c, sourceId);
			}

			OSCHelpers::handleControllableForOSCMessage(c, m);
			noFeedbackMap.remove(c);
		}
#else
		Controllable* c = OSCHelpers::findControllableAndHandleMessage(Engine::mainEngine, m);
#endif

		if (c == nullptr)
		{
			remoteControlListeners.call(&RemoteControlListener::processMessage, m);
			return;
		}
	}
}

void OSCRemoteControl::onContainerParameterChanged(Parameter* p)
{
	if (p == enabled || p == localPort) setupReceiver();
}

void OSCRemoteControl::oscMessageReceived(const OSCMessage& m)
{
	if (!enabled->boolValue()) return;
	processMessage(m);
}

void OSCRemoteControl::oscBundleReceived(const OSCBundle& b)
{
	if (!enabled->boolValue()) return;
	for (auto& m : b)
	{
		processMessage(m.getMessage());
	}
}

#if ORGANICUI_USE_SERVUS
void OSCRemoteControl::run()
{
	String nameToAdvertise = OrganicApplication::getInstance()->getApplicationName() + " - Remote Control";
	int portToAdvertise = 0;
	bool isEnabled = enabled->boolValue();

	while ((portToAdvertise != localPort->intValue() || isEnabled != enabled->boolValue()) && !threadShouldExit())
	{
		portToAdvertise = localPort->intValue();
		isEnabled = enabled->boolValue();

		servus.withdraw();

		if (isEnabled) servus.announce(portToAdvertise, nameToAdvertise.toStdString());


#if ORGANICUI_USE_WEBSERVER
		oscQueryServus.withdraw();
		if (isEnabled) oscQueryServus.announce(portToAdvertise, nameToAdvertise.toStdString());
#endif

		if (localPort->intValue() != portToAdvertise)
		{
			DBG("Name or port changed during advertise, readvertising");
		}
	}

	NLOG(niceName, "Zeroconf service created : " << nameToAdvertise << ":" << portToAdvertise);
}
#endif //SERVUS

#if ORGANICUI_USE_WEBSERVER
void OSCRemoteControl::setupServer()
{
	server.reset(new SimpleWebSocketServer());
	server->handler = this;
	server->addWebSocketListener(this);
	server->start(localPort->intValue(), "");
}

bool OSCRemoteControl::handleHTTPRequest(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
{
	juce::var data;
	if (juce::String(request->query_string).contains("HOST_INFO"))
	{
		juce::var extensionData(new juce::DynamicObject());
		extensionData.getDynamicObject()->setProperty("ACCESS", true);
		extensionData.getDynamicObject()->setProperty("CLIPMODE", false);
		extensionData.getDynamicObject()->setProperty("CRITICAL", false);
		extensionData.getDynamicObject()->setProperty("RANGE", true);
		extensionData.getDynamicObject()->setProperty("TAGS", false);
		extensionData.getDynamicObject()->setProperty("TYPE", true);
		extensionData.getDynamicObject()->setProperty("UNIT", false);
		extensionData.getDynamicObject()->setProperty("VALUE", true);
		extensionData.getDynamicObject()->setProperty("LISTEN", true);
		extensionData.getDynamicObject()->setProperty("PATH_ADDED", true);
		extensionData.getDynamicObject()->setProperty("PATH_REMOVED", true);
		extensionData.getDynamicObject()->setProperty("PATH_RENAMED", true);
		extensionData.getDynamicObject()->setProperty("PATH_CHANGED", false);

		data = new juce::DynamicObject();
		data.getDynamicObject()->setProperty("EXTENSIONS", extensionData);
		juce::String s = juce::String(ProjectInfo::projectName) + " - " + Engine::mainEngine->getDocumentTitle();
		data.getDynamicObject()->setProperty("NAME", s);
		data.getDynamicObject()->setProperty("OSC_PORT", localPort->intValue());
		data.getDynamicObject()->setProperty("OSC_TRANSPORT", "UDP");

		juce::var metaData(new juce::DynamicObject());
		data.getDynamicObject()->setProperty("METADATA", metaData);
		metaData.getDynamicObject()->setProperty("os", juce::SystemStats::getOperatingSystemName());
		metaData.getDynamicObject()->setProperty("version", ProjectInfo::versionString);
		metaData.getDynamicObject()->setProperty("versionNumber", ProjectInfo::versionNumber);
		if (fillHostInfoMetaDataFunc != nullptr) fillHostInfoMetaDataFunc(metaData);
	}
	else
	{
		ControllableContainer* cc = Engine::mainEngine;
		juce::String addr = request->path;
		if (addr.length() > 1) cc = Engine::mainEngine->getControllableContainerForAddress(addr, true, false, false);
		if (cc != nullptr) data = cc->getRemoteControlData();
	}


	juce::String dataStr = juce::JSON::toString(data);

	SimpleWeb::CaseInsensitiveMultimap header;
	header.emplace("Content-Length", juce::String(dataStr.length()).toStdString());
	header.emplace("Content-Type", "application/json");
	header.emplace("Accept-range", "bytes");
	header.emplace("Access-Control-Allow-Origin", "*");

	response->write(SimpleWeb::StatusCode::success_ok, header);
	*response << dataStr;

	return true;
}


void OSCRemoteControl::connectionOpened(const String& id)
{
	NLOG(niceName, "Got a connection from " << id);
	feedbackMap.set(id, Array<Controllable*>()); //Reset feedbacks
}

void OSCRemoteControl::messageReceived(const String& id, const String& message)
{
	var o = JSON::parse(message);
	if (o.isObject())
	{
		if (o.hasProperty("COMMAND"))
		{

			String command = o["COMMAND"];
			var data = o["DATA"];

			if (command == "ADD")
			{
				if (ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(data["address"].toString(), true))
				{
					cc->handleAddFromRemoteControl(data);
				}
			}
			else if (command == "REMOVE")
			{
				if (ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(data["address"].toString(), true))
				{
					cc->handleRemoveFromRemoteControl();
				}

			}
			else if (command == "RENAME")
			{
				if (ControllableContainer* cc = Engine::mainEngine->getControllableContainerForAddress(data["address"].toString(), true))
				{
					cc->setUndoableNiceName(data["name"]);
				}
			}

			if (Controllable* c = Engine::mainEngine->getControllableForAddress(data.toString()))
			{
				if (command == "LISTEN")
				{
					if (!feedbackMap.contains(id)) feedbackMap.set(id, Array<Controllable*>());
					(&(feedbackMap.getReference(id)))->addIfNotAlreadyThere(c);
				}
				else if (command == "IGNORE")
				{
					if (feedbackMap.contains(id)) (&(feedbackMap.getReference(id)))->removeAllInstancesOf(c);
				}
			}
		}
		else
		{
			NamedValueSet nvs = o.getDynamicObject()->getProperties();
			for (auto& nv : nvs)
			{
				if (Controllable* c = Engine::mainEngine->getControllableForAddress(nv.name.toString()))
				{
					if (c->type == Controllable::TRIGGER) ((Trigger*)c)->trigger();
					else ((Parameter*)c)->setValue(nv.value);
				}
			}
		}
	}
}

void OSCRemoteControl::dataReceived(const String& id, const MemoryBlock& data)
{
	OSCMessage m = OSCPacketParser(data.getData(), data.getSize()).readMessage();
	if (!m.isEmpty()) processMessage(m, id);
}

void OSCRemoteControl::connectionClosed(const String& id, int status, const String& reason)
{
	NLOG(niceName, "Connection close from " << id << " : " << status << " (" << reason << ")");
	feedbackMap.remove(id);
}

void OSCRemoteControl::connectionError(const String& id, const String& message)
{
	NLOGERROR(niceName, "Connection error from " << id << " : " << message);
	feedbackMap.remove(id);
}


void OSCRemoteControl::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	if (cc == &manualSendCC)
	{
		setupManualSender();
	}
}

void OSCRemoteControl::sendPathAddedFeedback(const String& path)
{
	if (server == nullptr) return;
	if (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)) return;

	var msg(new DynamicObject());
	msg.getDynamicObject()->setProperty("COMMAND", "PATH_ADDED");
	msg.getDynamicObject()->setProperty("DATA", path);
	server->send(JSON::toString(msg));
}

void OSCRemoteControl::sendPathRemovedFeedback(const String& path)
{
	if (server == nullptr) return;
	if (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)) return;

	var msg(new DynamicObject());
	msg.getDynamicObject()->setProperty("COMMAND", "PATH_REMOVED");
	msg.getDynamicObject()->setProperty("DATA", path);
	server->send(JSON::toString(msg));
}

void OSCRemoteControl::sendPathNameChangedFeedback(const String& oldPath, const String& newPath)
{
	if (server == nullptr) return;
	if (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)) return;

	var msg(new DynamicObject());
	msg.getDynamicObject()->setProperty("COMMAND", "PATH_RENAMED");
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("OLD", oldPath);
	data.getDynamicObject()->setProperty("NEW", newPath);
	msg.getDynamicObject()->setProperty("DATA", data);
	server->send(JSON::toString(msg));

}

void OSCRemoteControl::newMessage(const ContainerAsyncEvent& e)
{
	if (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)) return;

	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		//OSCQuery
		HashMap<String, Array<Controllable*>, DefaultHashFunctions, CriticalSection>::Iterator it(feedbackMap);
		while (it.next())
		{
			if (it.getValue().contains(e.targetControllable))
			{
				sendOSCQueryFeedback(e.targetControllable);
			}
		}

		//Manual
		sendManualFeedbackForControllable(e.targetControllable);

	}

}

void OSCRemoteControl::sendOSCQueryFeedback(Controllable* c, const String& excludeId)
{
	if (c == nullptr) return;

	OSCMessage m = OSCHelpers::getOSCMessageForControllable(c);
	StringArray ex = excludeId;
	if (noFeedbackMap.contains(c)) ex.add(noFeedbackMap[c]);
	sendOSCQueryFeedback(m, ex);

}

void OSCRemoteControl::sendOSCQueryFeedback(const OSCMessage& m, StringArray excludes)
{
	if (server == nullptr) return;

	OSCPacketPacker packer;
	if (packer.writeMessage(m))
	{
		MemoryBlock b(packer.getData(), packer.getDataSize());
		server->sendExclude(b, excludes);
	}

	if (logOutgoing->boolValue()) NLOG(niceName, "Sent to OSCQuery : " << OSCHelpers::messageToString(m));
}
#endif

void OSCRemoteControl::sendAllManualFeedback()
{
	if (!manualSendCC.enabled->boolValue()) return;

	Array<WeakReference<Controllable>> allControllables = Engine::mainEngine->getAllControllables(true);
	for (auto& c : allControllables)
	{
		sendManualFeedbackForControllable(c);
	}
}

void OSCRemoteControl::sendManualFeedbackForControllable(Controllable* c)
{
	if (!manualSendCC.enabled->boolValue()) return;
	if (c == nullptr || c->hideInRemoteControl) return;
	OSCMessage m = OSCHelpers::getOSCMessageForControllable(c);

	manualSender.send(m);

	if (logOutgoing->boolValue()) NLOG(niceName, "Sent to manual OSC  : " << OSCHelpers::messageToString(m));
}
