/*
  ==============================================================================

	OSCRemoteControl.cpp
	Created: 23 Apr 2018 5:00:30pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"
juce_ImplementSingleton(OSCRemoteControl)

#if ORGANICUI_USE_WEBSERVER
#include "OSCPacketHelper.h"
#include "OSCRemoteControl.h"
#endif

#ifndef ORGANIC_REMOTE_CONTROL_PORT
#define ORGANIC_REMOTE_CONTROL_PORT 42000
#endif

using namespace juce;

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
	, localPort(nullptr)
{

	saveAndLoadRecursiveData = true; //can be useful when app include other settings there

	enabled->setValue(false);

	localPort = addIntParameter("Local Port", "Local port to connect to for global control over the application", ORGANIC_REMOTE_CONTROL_PORT, 1, 65535);
	logIncoming = addBoolParameter("Log Incoming", "If checked, this will log incoming messages", false);
	logOutgoing = addBoolParameter("Log Outgoing", "If checked, this will log outgoing messages", false);

	sendFeedbackOnListen = addBoolParameter("Send update on listen", "Sends feedback with the current controllable value when a client sends a LISTEN command", false);
	enableSendLogFeedback = addBoolParameter("Send Log Feedback", "If checked, this will send log messages to connected clients", false);

	receiver.addListener(this);
	receiver.registerFormatErrorHandler(&OSCHelpers::logOSCFormatError);

	manualAddress = manualSendCC.addStringParameter("Address", "Address to send to", "127.0.0.1");
	manualPort = manualSendCC.addIntParameter("Port", "Port to send to", ORGANIC_REMOTE_CONTROL_PORT + 1, 1, 65535);

	manualSendCC.enabled->setDefaultValue(false);
	addChildControllableContainer(&manualSendCC);

#if ORGANICUI_USE_WEBSERVER
#endif
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
	if (WarningReporter::getInstanceWithoutCreating() != nullptr) WarningReporter::getInstance()->removeAsyncWarningReporterListener(this);
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
	//Engine::mainEngine->removeAsyncContainerListener(this);
	Engine::mainEngine->removeControllableContainerListener(this);
	WarningReporter::getInstance()->removeAsyncWarningReporterListener(this);

	bool shouldListen = false;
	if (!enabled->boolValue()) return;
	if (manualSendCC.enabled->boolValue()) shouldListen = true;
	if (receiverIsConnected) shouldListen = true;

	//if (shouldListen) Engine::mainEngine->addAsyncContainerListener(this);
	if (shouldListen)
	{
		Engine::mainEngine->addControllableContainerListener(this);
		WarningReporter::getInstance()->addAsyncWarningReporterListener(this);
	}
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

			MessageManager::getInstance()->callAsync([f]() { Engine::mainEngine->loadDocument(f); });
		}
	}
	else if (add == "/loadFile")
	{
		if (m.size() < 1) LOGWARNING("Cannot load file, no argument provided");
		else
		{
			if (!m[0].isString())
			{
				LOGWARNING("Cannot load file, argument #0 is not a string");
				return;
			}

			var data = JSON::parse(m[0].getString());
			if (!data.isObject())
			{
				LOGWARNING("Cannot load file, argument #0 is not a valid JSON object");
				return;
			}

			MessageManager::getInstance()->callAsync([data]() {
				Engine::mainEngine->loadJSONData(data, nullptr);
				Engine::mainEngine->setFile(File());
				});
		}
	}
	else if (add == "/newFile")
	{
		MessageManager::getInstance()->callAsync([]() { Engine::mainEngine->createNewGraph(); });
	}
	else if (add == "/saveFile")
	{
		if (m.size() >= 1 && m[0].isString())
		{
			File f = File::getSpecialLocation(File::userDocumentsDirectory).getNonexistentChildFile(getApp().getApplicationName() + "/" + m[0].getString(), Engine::mainEngine->fileExtension, true);

			MessageManager::callAsync([f]() { Engine::mainEngine->saveAsAsync(f, false, false, false, [](int result) {}); });
		}
		else
		{
			if (Engine::mainEngine->getFile().existsAsFile()) Engine::mainEngine->saveAsync(false, false, nullptr);
			else
			{
				File f = File::getSpecialLocation(File::userDocumentsDirectory).getNonexistentChildFile(getApp().getApplicationName() + "/default", Engine::mainEngine->fileExtension, true);

				MessageManager::getInstance()->callAsync([f]() { Engine::mainEngine->saveAsAsync(f, false, false, false, [](int result) {}); });
			}
		}
		LOG("File saved.");
	}
	else if (add == "/closeApp")
	{
		MessageManager::callAsync([]() {OrganicApplication::quit(); });
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

		if (c == nullptr)
		{
			String addr = m.getAddressPattern().toString();
			if (addr.contains("/attributes/"))
			{
				String splitString = addr.replace("/attributes/", " ");
				StringArray split = StringArray::fromTokens(splitString, true);
				if (split.size() == 2)
				{
					if (Controllable* c = Engine::mainEngine->getControllableForAddress(split[0]))
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

						c->setAttribute(split[1], OSCHelpers::getBoolArg(m[0]));

						noFeedbackMap.remove(c);

					}
				}
			}
		}

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

			bool handled = false;
			if (c->parentContainer != nullptr) handled = c->parentContainer->handleRemoteControlData(c, m, sourceId);
			if (!handled) OSCHelpers::handleControllableForOSCMessage(c, m);

			noFeedbackMap.remove(c);
		}
#else
		Controllable* c = OSCHelpers::findControllableAndHandleMessage(Engine::mainEngine, m);
#endif

		if (c == nullptr)
		{
			bool handled = false;
			if (ControllableContainer* cc = OSCHelpers::findParentContainer(Engine::mainEngine, m.getAddressPattern().toString()))
			{
				handled = cc->handleRemoteControlData(m, sourceId);
			}

			if (!handled) remoteControlListeners.call(&RemoteControlListener::processMessage, m, sourceId);

		}
	}
}

void OSCRemoteControl::onContainerParameterChanged(Parameter* p)
{
	if (p == enabled || p == localPort) setupReceiver();
#if ORGANICUI_USE_WEBSERVER
	else if (p == enableSendLogFeedback)
	{
		if (enableSendLogFeedback->boolValue())  CustomLogger::getInstance()->addLogListener(this);
		else CustomLogger::getInstance()->removeLogListener(this);
	}
#endif
}

void OSCRemoteControl::oscMessageReceived(const OSCMessage& m)
{
	if (!enabled->boolValue() || Engine::mainEngine->isClearing) return;
	processMessage(m);
}

void OSCRemoteControl::oscBundleReceived(const OSCBundle& b)
{
	if (!enabled->boolValue() || Engine::mainEngine->isClearing) return;
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
	bool isEnabled = enabled->boolValue() && !Engine::mainEngine->isClearing;

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
	else if (juce::String(request->path) == "/sessionFile")
	{
		data = Engine::mainEngine->getJSONData();
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
	remoteControlListeners.call(&RemoteControlListener::clientConnected, id);
	feedbackMap.set(id, Array<Controllable*>()); //Reset feedbacks

	for (auto& wt : WarningReporter::getInstance()->targets)
	{
		if (wt == nullptr || wt.wasObjectDeleted()) continue;
		sendPersistentWarningFeedback(wt);
	}

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
			else if (command == "LOAD")
			{
				var fileData = JSON::parse(data["data"]);

				if (fileData.isObject())
				{
					if (ControllableContainer* cc = data["address"] == "/" ? Engine::mainEngine : Engine::mainEngine->getControllableContainerForAddress(data["address"].toString(), true))
					{
						MessageManager::callAsync([cc, fileData]() { cc->handleLoadFromRemoteControl(fileData); });
					}
				}
			}
			else if (command == "SAVE")
			{
				if (ControllableContainer* cc = data["address"] == "/" ? Engine::mainEngine : Engine::mainEngine->getControllableContainerForAddress(data["address"].toString(), true))
				{
					var saveData = cc->handleSaveFromRemoteControl();
					if (saveData.isObject())
					{
						var msg(new DynamicObject());
						msg.getDynamicObject()->setProperty("COMMAND", "SAVE");
						var datamsg(new DynamicObject());
						datamsg.getDynamicObject()->setProperty("address", data["address"]);
						datamsg.getDynamicObject()->setProperty("data", saveData);
						msg.getDynamicObject()->setProperty("DATA", datamsg);
						server->sendTo(JSON::toString(msg), id);
					}
				}
			}
			else if (command == "UNDO")
			{
				UndoMaster::getInstance()->undo();
			}
			else if (command == "REDO")
			{
				UndoMaster::getInstance()->redo();
			}
			else if (command == "LOG")
			{
				if (data.hasProperty("type"))
				{
					if (data["type"] == "info") NLOG(niceName, data["message"].toString());
					else if (data["type"] == "warning") NLOGWARNING(niceName, data["message"].toString());
					else if (data["type"] == "error") NLOGERROR(niceName, data["message"].toString());
					else NLOG(niceName, data["message"].toString());
				}
				else NLOG(niceName, data.toString());

			}
			String cAddress = "";
			if (data.isString()) cAddress = data.toString();
			else if (data.isArray() && data.size() > 0) cAddress = data[0].toString();
			else if (data.isObject() && data.hasProperty("address")) cAddress = data["address"].toString();

			if (Controllable* c = Engine::mainEngine->getControllableForAddress(cAddress))
			{
				if (command == "LISTEN")
				{
					if (!feedbackMap.contains(id)) feedbackMap.set(id, Array<Controllable*>());
					(&(feedbackMap.getReference(id)))->addIfNotAlreadyThere(c);
					bool sendFeedback = sendFeedbackOnListen->boolValue();
					if (data.isArray() && data.size() > 1) sendFeedback = data[1];
					else if (data.isObject() && data.hasProperty("sendFeedback")) sendFeedback = data["sendFeedback"];
					if (sendFeedback) sendOSCQueryFeedback(c);
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
				else if (nv.name.toString().startsWith("undoable:") && nv.value.size() == 2)
				{
					String cName = nv.name.toString().fromFirstOccurrenceOf(":", false, false);
					if (Parameter* p = dynamic_cast<Parameter*>(Engine::mainEngine->getControllableForAddress(cName)))
					{
						if (nv.value.size() == 2)
						{
							p->setUndoableValue(nv.value[0], nv.value[1]);
						}
					}
				}
				else if (nv.name.toString().startsWith("undoables:"))
				{
					String actionName = nv.name.toString().fromFirstOccurrenceOf(":", false, false);
					if (nv.value.isObject())
					{
						NamedValueSet paramVS = nv.value.getDynamicObject()->getProperties();
						Array<UndoableAction*> actions;
						for (auto& pnv : paramVS)
						{
							if (Parameter* p = dynamic_cast<Parameter*>(Engine::mainEngine->getControllableForAddress(pnv.name.toString())))
							{
								if (pnv.value.size() == 2)
								{
									actions.add(p->setUndoableValue(pnv.value[0], pnv.value[1], true));
								}
							}
						}
						UndoMaster::getInstance()->performActions(actionName.isEmpty() ? "Set " + String(actions.size()) + " values from remote" : actionName, actions);
					}

				}
				else if (nv.name.toString().contains("/attributes/"))
				{
					StringArray split = StringArray::fromTokens(nv.name.toString(), "/attributes/", "\"");
					if (split.size() == 2)
					{
						if (Controllable* c = Engine::mainEngine->getControllableForAddress(split[0]))
						{
							c->setAttribute(split[1], nv.value);
						}
					}
				}
			}
		}
	}
}

void OSCRemoteControl::dataReceived(const String& id, const MemoryBlock& data)
{
	OSCMessage m = OSCPacketParser(data.getData(), data.getSize()).readMessage();
	processMessage(m, id);
}

void OSCRemoteControl::connectionClosed(const String& id, int status, const String& reason)
{
	remoteControlListeners.call(&RemoteControlListener::clientDisconnected, id, reason);
	NLOG(niceName, "Connection close from " << id << " : " << status << " (" << reason << ")");
	feedbackMap.remove(id);
}

void OSCRemoteControl::connectionError(const String& id, const String& message)
{
	remoteControlListeners.call(&RemoteControlListener::clientDisconnected, id, message);
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

void OSCRemoteControl::sendPathChangedFeedback(const juce::String& path)
{
	if (server == nullptr) return;
	if (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)) return;

	var msg(new DynamicObject());
	msg.getDynamicObject()->setProperty("COMMAND", "PATH_RENAMED");
	msg.getDynamicObject()->setProperty("DATA", path);
	server->send(JSON::toString(msg));
}

bool OSCRemoteControl::hasClient(const String& id)
{
	if (server == nullptr) return false;
	return server->connectionMap.contains(id);
}

void OSCRemoteControl::newMessage(const CustomLogger::LogEvent& e)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;
	if (e.log == nullptr) return;
	if (enableSendLogFeedback != nullptr && !enableSendLogFeedback->boolValue()) return;
	sendLogFeedback(e.log->getSeverityName(), e.log->source, e.log->getContent());
}

void OSCRemoteControl::sendLogFeedback(const String& type, const String& source, const String& message)
{
	var msg(new DynamicObject());
	msg.getDynamicObject()->setProperty("COMMAND", "LOG");
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("type", type);
	data.getDynamicObject()->setProperty("source", source);
	data.getDynamicObject()->setProperty("message", message);
	msg.getDynamicObject()->setProperty("DATA", data);
	server->send(JSON::toString(msg));
}

void OSCRemoteControl::sendPersistentWarningFeedback(WeakReference<WarningTarget> wt, String address, WarningReporter::WarningReporterEvent::Type type)
{
	if (server == nullptr) return;
	if (wt == nullptr || wt.wasObjectDeleted()) return;
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;

	String path = address;
	String name;
	if (Controllable* c = dynamic_cast<Controllable*>(wt.get()))
	{
		name = c->niceName;
		if (path.isEmpty()) path = c->getControlAddress();
	}
	else if (ControllableContainer* cc = dynamic_cast<ControllableContainer*>(wt.get()))
	{
		name = cc->niceName;
		if (path.isEmpty()) path = cc->getControlAddress();
	}

	if (path.isEmpty()) return;

	String wMessage = type == WarningReporter::WarningReporterEvent::Type::WARNING_REGISTERED ? wt->getWarningMessage() : "";

	var msg(new DynamicObject());
	msg.getDynamicObject()->setProperty("COMMAND", "WARNING");
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("source", path);
	if (name.isNotEmpty()) data.getDynamicObject()->setProperty("name", name);
	data.getDynamicObject()->setProperty("message", wMessage);
	msg.getDynamicObject()->setProperty("DATA", data);
	server->send(JSON::toString(msg));
}

void OSCRemoteControl::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	EnablingControllableContainer::controllableFeedbackUpdate(cc, c);

	if (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)) return;

	//OSCQuery
	juce::HashMap<String, Array<Controllable*>, DefaultHashFunctions, CriticalSection>::Iterator it(feedbackMap);
	while (it.next())
	{
		if (it.getValue().contains(c))
		{
			sendOSCQueryFeedback(c);
		}
	}

	//Manual
	sendManualFeedbackForControllable(c);

}

void OSCRemoteControl::controllableStateUpdate(ControllableContainer* cc, Controllable* c)
{
	EnablingControllableContainer::controllableStateUpdate(cc, c);

	if (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)) return;

	//OSCQuery
	juce::HashMap<String, Array<Controllable*>, DefaultHashFunctions, CriticalSection>::Iterator it(feedbackMap);
	while (it.next())
	{
		if (it.getValue().contains(c))
		{
			sendOSCQueryStateFeedback(c);
		}
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

void OSCRemoteControl::sendOSCQueryStateFeedback(Controllable* c, const juce::String& excludeId)
{
	if (c == nullptr) return;

	OSCMessage m(c->getControlAddress() + "/attributes/enabled");
	m.addBool(c->enabled);
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

void OSCRemoteControl::sendOSCQueryFeedbackTo(const juce::OSCMessage& m, juce::StringArray ids)
{
	if (server == nullptr) return;

	OSCPacketPacker packer;
	if (packer.writeMessage(m))
	{
		MemoryBlock b(packer.getData(), packer.getDataSize());
		for (auto& id : ids) server->sendTo(b, id);
	}

	if (logOutgoing->boolValue()) NLOG(niceName, "Sent to OSCQuery : " << OSCHelpers::messageToString(m));

}


void OSCRemoteControl::newMessage(const WarningReporter::WarningReporterEvent& e)
{
	sendPersistentWarningFeedback(e.target, e.targetAddress, e.type);
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
