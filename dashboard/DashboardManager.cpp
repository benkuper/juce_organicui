/*
  ==============================================================================

    DashboardManager.cpp
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(DashboardManager)

DashboardManager::DashboardManager() :
	BaseManager("Dashboards")
#if ORGANICUI_USE_SERVUS
	, Thread("Global Zeroconf")
	, servus("_http._tcp")
#endif
{
	editMode = addBoolParameter("Edit Mode", "If checked, items are editable. If not, items are normally usable", true);
	snapping = addBoolParameter("Snapping", "If checked, items are automatically aligned when dragging them closed to other ones", true);
}

DashboardManager::~DashboardManager()
{
#if ORGANICUI_USE_WEBSERVER
	server.reset();
#endif

#if ORGANICUI_USE_SERVUS
	signalThreadShouldExit();
	waitForThreadToExit(1000);
#endif

	DashboardItemFactory::deleteInstance();
}

#if ORGANICUI_USE_WEBSERVER

void DashboardManager::setupServer()
{
	server.reset();
	if (isCurrentlyLoadingData) return;

	if (!ProjectSettings::getInstance()->enableServer->boolValue())
	{
		LOG("Dashboard server is not running");
		#if ORGANICUI_USE_SERVUS
				signalThreadShouldExit();
				waitForThreadToExit(1000);
		#endif

		return;
	}

	int port = ProjectSettings::getInstance()->serverPort->intValue();

	server.reset(new SimpleWebSocket());
	server->rootPath = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile(OrganicApplication::getInstance()->getApplicationName() + "/dashboard");
	server->addWebSocketListener(this);
	server->start(port);

#if ORGANICUI_USE_SERVUS
	setupZeroconf();
#endif

	LOG("Dashboard server is running on port " << port);

}

void DashboardManager::connectionOpened(const String& id)
{
	LOG("New browser connection to the Dashboard from " << id);
	if (server == nullptr) return;

	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("dataType", "all");
	data.getDynamicObject()->setProperty("appName", OrganicApplication::getInstance()->getApplicationName());

	var iData;
	for (auto& d : items)
	{
		iData.append(d->getServerData());
	}
	data.getDynamicObject()->setProperty("items", iData);
	String s = JSON::toString(data,true);
	server->sendTo(s, id);
}

void DashboardManager::messageReceived(const String& id, const String& message)
{
	var data;
	Result result =  JSON::parse(message, data);

	if (result.failed())
	{
		DBG("Error parsing: " << message << ", error : " << result.getErrorMessage());
		return;
	}
	
	
	String add = data.getProperty("controlAddress", "");
	if (add.isNotEmpty())
	{
		Controllable * c = Engine::mainEngine->getControllableForAddress(add);
		if (c == nullptr)
		{
			DBG("Controllable not found for address " << add);
		}
		else
		{
			switch (c->type)
			{
			case Controllable::TRIGGER:
				((Trigger*)c)->trigger();
				break;

			default: //Parameter
			{
				var val = data.getProperty("value", var());
				if (!val.isVoid())
				{
					((Parameter*)c)->setValue(val);
				}
			}
			break;
			}
		}
	}
}

void DashboardManager::connectionClosed(const String& id, int status, const String& reason)
{
	LOG("Connection to the Dashboard closed by " << id);
}
#endif //ORGANICUI_USE_WEBSERVER


void DashboardManager::addItemInternal(Dashboard* item, var data)
{
	item->addDashboardListener(this);
}

void DashboardManager::removeItemInternal(Dashboard* item)
{
	item->removeDashboardListener(this);
}

void DashboardManager::itemDataFeedback(var data)
{
#if ORGANICUI_USE_WEBSERVER
	if (server != nullptr)
	{
		data.getDynamicObject()->setProperty("dataType", "feedback");
		server->send(JSON::toString(data));
	}
#endif
}

void DashboardManager::afterLoadJSONDataInternal()
{
#if ORGANICUI_USE_WEBSERVER
	setupServer();
#endif
}


#if ORGANICUI_USE_SERVUS
void DashboardManager::setupZeroconf()
{
	if(ProjectSettings::getInstanceWithoutCreating() == nullptr || ProjectSettings::getInstance()->serverPort == nullptr) return;

	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;
	if (!isThreadRunning()) startThread();
}

void DashboardManager::run()
{
	String nameToAdvertise = OrganicApplication::getInstance()->getApplicationName() + " - Dashboard";
	int port = ProjectSettings::getInstance()->serverPort->intValue();
	int portToAdvertise = 0;
	while (portToAdvertise != port && !threadShouldExit())
	{
		portToAdvertise = port;
		servus.withdraw();
		servus.announce(portToAdvertise, nameToAdvertise.toStdString());

		if (port != portToAdvertise)
		{
			DBG("Name or port changed during advertise, readvertising");
		}
	}

	NLOG(niceName, "Zeroconf service created : " << nameToAdvertise << ":" << portToAdvertise);
}
#endif