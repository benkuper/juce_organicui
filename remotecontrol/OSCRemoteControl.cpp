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

	localPort = addIntParameter("Local Port", "Local port to connect to for global control over the application", OSC_REMOTE_CONTROL_PORT, 1, 65535);
	logIncoming = addBoolParameter("Log Incoming", "If checked, this will log incoming messages", false);
	logOutgoing = addBoolParameter("Log Outgoing", "If checked, this will log outgoing messages", false);

	receiver.addListener(this);
	receiver.registerFormatErrorHandler(&OSCHelpers::logOSCFormatError);

	manualAddress = manualSendCC.addStringParameter("Address", "Address to send to", "127.0.0.1");
	manualPort = manualSendCC.addIntParameter("Port", "Port to send to", OSC_REMOTE_CONTROL_PORT + 1, 1, 65535);

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

	if (!enabled->boolValue()) return;

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
	Engine::mainEngine->removeAsyncContainerListener(this);

	bool shouldListen = false;
	if (!enabled->boolValue()) return;
	if (manualSendCC.enabled->boolValue()) shouldListen = true;
	if (receiverIsConnected) shouldListen = true;

	if (shouldListen) Engine::mainEngine->addAsyncContainerListener(this);
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
	while (portToAdvertise != localPort->intValue() && !threadShouldExit())
	{
		portToAdvertise = localPort->intValue();
		servus.withdraw();
		servus.announce(portToAdvertise, nameToAdvertise.toStdString());


#if ORGANICUI_USE_WEBSERVER
		oscQueryServus.withdraw();
		oscQueryServus.announce(portToAdvertise, nameToAdvertise.toStdString());
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
	var data;
	if (String(request->query_string).contains("HOST_INFO"))
	{
		var extensionData(new DynamicObject());
		extensionData.getDynamicObject()->setProperty("ACCESS", true);
		extensionData.getDynamicObject()->setProperty("CLIPMODE", false);
		extensionData.getDynamicObject()->setProperty("CRITICAL", false);
		extensionData.getDynamicObject()->setProperty("RANGE", true);
		extensionData.getDynamicObject()->setProperty("TAGS", false);
		extensionData.getDynamicObject()->setProperty("TYPE", true);
		extensionData.getDynamicObject()->setProperty("UNIT", false);
		extensionData.getDynamicObject()->setProperty("VALUE", true);
		extensionData.getDynamicObject()->setProperty("LISTEN", true);

		data = new DynamicObject();
		data.getDynamicObject()->setProperty("EXTENSIONS", extensionData);
		String s = String(ProjectInfo::projectName) + " - " + Engine::mainEngine->getDocumentTitle();
		data.getDynamicObject()->setProperty("NAME", s);
		data.getDynamicObject()->setProperty("OSC_PORT", localPort->intValue());
		data.getDynamicObject()->setProperty("OSC_TRANSPORT", "UDP");
	}
	else
	{
		data = getOSCQueryDataForContainer(Engine::mainEngine);
	}


	String dataStr = JSON::toString(data);

	SimpleWeb::CaseInsensitiveMultimap header;
	header.emplace("Content-Length", String(dataStr.length()).toStdString());
	header.emplace("Content-Type", "application/json");
	header.emplace("Accept-range", "bytes");
	header.emplace("Access-Control-Allow-Origin", "*");

	response->write(SimpleWeb::StatusCode::success_ok, header);
	*response << dataStr;

	return true;
}

var OSCRemoteControl::getOSCQueryDataForContainer(ControllableContainer* cc)
{
	var data(new DynamicObject());

	data.getDynamicObject()->setProperty("DESCRIPTION", cc->niceName);
	data.getDynamicObject()->setProperty("FULL_PATH", cc->getControlAddress());
	data.getDynamicObject()->setProperty("ACCESS", 0); //container, no value directly associated

	var contentData(new DynamicObject());

	for (auto& childC : cc->controllables)
	{
		if (childC->hideInRemoteControl) continue;
		contentData.getDynamicObject()->setProperty(childC->shortName, getOSCQueryDataForControllable(childC));
	}

	for (auto& childCC : cc->controllableContainers)
	{
		if (childCC->hideInRemoteControl) continue;
		contentData.getDynamicObject()->setProperty(childCC->shortName, getOSCQueryDataForContainer(childCC));
	}

	data.getDynamicObject()->setProperty("CONTENTS", contentData);

	return data;
}

var OSCRemoteControl::getOSCQueryDataForControllable(Controllable* c)
{
	var data(new DynamicObject());

	data.getDynamicObject()->setProperty("DESCRIPTION", c->niceName);
	data.getDynamicObject()->setProperty("FULL_PATH", c->getControlAddress());
	data.getDynamicObject()->setProperty("ACCESS", c->isControllableFeedbackOnly ? 1 : 3);

	String typeString = "";
	var value;
	var range;

	if (c->type == Controllable::TRIGGER) typeString = "N";
	else
	{
		Parameter* p = (Parameter*)c;

		if (p->hasRange())
		{
			if (p->isComplex())
			{
				for (int i = 0; i < p->minimumValue.size(); i++)
				{
					var rData(new DynamicObject());
					rData.getDynamicObject()->setProperty("MIN", p->minimumValue[i]);
					rData.getDynamicObject()->setProperty("MAX", p->maximumValue[i]);
					range.append(rData);
				}
			}
			else
			{
				var rData(new DynamicObject());
				rData.getDynamicObject()->setProperty("MIN", p->minimumValue);
				rData.getDynamicObject()->setProperty("MAX", p->maximumValue);
				range.append(rData);
			}

		}

		switch (p->type)
		{
		case Parameter::BOOL:
			value.append(p->boolValue());
			typeString = "T";
			break;

		case Parameter::INT:
		{
			//add range
			value.append(p->intValue());

			typeString = "i";
		}
		break;

		case Parameter::FLOAT:
			value.append(p->floatValue());
			typeString = "f";
			break;

		case Parameter::STRING:
			typeString = "s";
			value.append(p->stringValue());
			break;

		case Parameter::POINT2D:
			value.append(((Point2DParameter*)p)->x);
			value.append(((Point2DParameter*)p)->y);
			typeString = "ff";
			break;

		case Parameter::POINT3D:
			value.append(((Point3DParameter*)p)->x);
			value.append(((Point3DParameter*)p)->y);
			value.append(((Point3DParameter*)p)->z);
			typeString = "fff";
			break;

		case Parameter::COLOR:
			value.append(((ColorParameter*)p)->getColor().toString());
			typeString = "r";
			break;


		case Parameter::ENUM:
		{
			typeString = "s";

			EnumParameter* ep = (EnumParameter*)p;
			value.append(ep->getValueKey());

			StringArray keys = ep->getAllKeys();
			var enumRange;
			for (auto& k : keys) enumRange.append(k);
			var rData(new DynamicObject());
			rData.getDynamicObject()->setProperty("VALS", enumRange);
			range.append(rData);
		}
		break;

		default:
			break;
		}
	}

	data.getDynamicObject()->setProperty("TYPE", typeString);
	if (!value.isVoid()) data.getDynamicObject()->setProperty("VALUE", value);
	if (!range.isVoid()) data.getDynamicObject()->setProperty("RANGE", range);

	return data;
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
		String command = o["COMMAND"];
		String data = o["DATA"];

		if (Controllable* c = Engine::mainEngine->getControllableForAddress(data))
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
}

void OSCRemoteControl::dataReceived(const String& id, const MemoryBlock& data)
{
	OSCMessage m = OSCPacketParser(data.getData(), data.getSize()).readMessage();
	if (!m.isEmpty()) processMessage(m);
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

void OSCRemoteControl::newMessage(const ContainerAsyncEvent& e)
{
	if (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing) return;

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
	OSCPacketPacker packer;
	if (packer.writeMessage(m))
	{
		MemoryBlock b(packer.getData(), packer.getDataSize());

		StringArray ex = excludeId;
		if (noFeedbackMap.contains(c)) ex.add(noFeedbackMap[c]);
		server->sendExclude(b, ex);
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
