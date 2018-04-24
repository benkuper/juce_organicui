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
{
	localPort = addIntParameter("Local Port", "Local port to connect to for global control over the application", 42000, 1024, 65535);

	receiver.addListener(this);

	setupReceiver();
}

OSCRemoteControl::~OSCRemoteControl()
{
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
		//if (!isThreadRunning()) startThread();
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

void OSCRemoteControl::processMessage(const OSCMessage & m)
{
	String add = m.getAddressPattern().toString();

	Controllable * c = Engine::mainEngine->getControllableForAddress(add);

	if (c == nullptr)
	{
		NLOG(niceName, "No target found for " << add);
		return;
	}

	if (c->type == Controllable::TRIGGER)
	{
		if (m.size() == 0 || (m.size() > 0 && OSCHelpers::getIntArg(m[0]) > 0)) static_cast<Trigger *>(c)->trigger();
	} else
	{
		Parameter * p = dynamic_cast<Parameter *>(c);
		jassert(p != nullptr);
		switch (p->type)
		{

		case Controllable::BOOL:
			if (m.size() == 0) NLOG(niceName, "Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(OSCHelpers::getIntArg(m[0]) > 0);
			break;

		case Controllable::INT:
			if (m.size() == 0) NLOG(niceName, "Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(OSCHelpers::getIntArg(m[0]));
			break;

		case Controllable::FLOAT:
			if (m.size() == 0) NLOG(niceName, "Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(OSCHelpers::getFloatArg(m[0]));
			break;

		case Controllable::STRING:
			if (m.size() == 0) NLOG(niceName, "Parameter " << p->niceName << " requires 1 argument");
			else p->setValue(OSCHelpers::getStringArg(m[0]));
			break;

		case Controllable::COLOR:
			if (m.size() < 4) NLOG(niceName, "Parameter " << p->niceName << " requires at least 4 argument");
			else static_cast<ColorParameter *>(p)->setColor(Colour::fromFloatRGBA(OSCHelpers::getFloatArg(m[0]), OSCHelpers::getFloatArg(m[1]), OSCHelpers::getFloatArg(m[2]), m.size() >= 4 ? OSCHelpers::getFloatArg(m[0]) : 1));
			break;

		default:
			NLOG(niceName, "Type not handled : " << c->getTypeString());
			break;
		};
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
