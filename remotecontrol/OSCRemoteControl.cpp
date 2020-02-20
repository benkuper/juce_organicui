/*
  ==============================================================================

	OSCRemoteControl.cpp
	Created: 23 Apr 2018 5:00:30pm
	Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(OSCRemoteControl)

OSCRemoteControl::OSCRemoteControl() :
	EnablingControllableContainer("OSC Remote Control")
#if ORGANICUI_USE_SERVUS
	,Thread("Global Zeroconf")
	,servus("_osc._udp")
#endif
{

	saveAndLoadRecursiveData = true; //can be useful when app include other settings there

	enabled->setValue(false);

	localPort = addIntParameter("Local Port", "Local port to connect to for global control over the application", 42000, 1024, 65535);

	receiver.addListener(this);
	receiver.registerFormatErrorHandler(&OSCHelpers::logOSCFormatError);

	//setupReceiver();
}

OSCRemoteControl::~OSCRemoteControl()
{
#if ORGANICUI_USE_SERVUS
	signalThreadShouldExit();
	waitForThreadToExit(1000);
#endif
}

void OSCRemoteControl::setupReceiver()
{

	//if (receiveCC == nullptr) return;

	receiver.disconnect();

	if (!enabled->boolValue()) return;

	//if (!receiveCC->enabled->boolValue()) return;
	bool result = receiver.connect(localPort->intValue());

	if (result)
	{
		NLOG(niceName, "Now receiving on port : " + localPort->stringValue());
#if ORGANICUI_USE_SERVUS
		setupZeroconf();
#endif
	} else
	{
		NLOGERROR(niceName, "Error binding port " + localPort->stringValue());
	}


	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);

	Array<String> ips;
	for (auto &a : ad) ips.add(a.toString());
	ips.sort();
	String s = "Local IPs:";
	for (auto &ip : ips) s += String("\n > ") + ip;

	NLOG(niceName, s);
}

#if ORGANICUI_USE_SERVUS
void OSCRemoteControl::setupZeroconf()
{
	if (Engine::mainEngine->isClearing || localPort == nullptr) return;
	if(!isThreadRunning()) startThread();
}
#endif

void OSCRemoteControl::processMessage(const OSCMessage& m)
{
	String add = m.getAddressPattern().toString();

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
		Engine::mainEngine->save(false, false);
	}
	else if (add == "/closeApp")
	{
		MessageManagerLock mmLock;
		OrganicApplication::quit();
	}
	else
	{
		Controllable* c = OSCHelpers::findControllableAndHandleMessage(Engine::mainEngine, m);

		if (c == nullptr)
		{
			remoteControlListeners.call(&RemoteControlListener::processMessage, m);
			return;
		}
	}
}

void OSCRemoteControl::onContainerParameterChanged(Parameter * p)
{
	if (p == enabled || p == localPort) setupReceiver();
}

void OSCRemoteControl::oscMessageReceived(const OSCMessage & m)
{
	if (!enabled->boolValue()) return;
	processMessage(m);
}

void OSCRemoteControl::oscBundleReceived(const OSCBundle & b)
{
	if (!enabled->boolValue()) return;
	for (auto &m : b)
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

		if (localPort->intValue() != portToAdvertise)
		{
			DBG("Name or port changed during advertise, readvertising");
		}
	}

	NLOG(niceName, "Zeroconf service created : " << nameToAdvertise << ":" << portToAdvertise);
}
#endif