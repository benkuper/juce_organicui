/*
  ==============================================================================

	DashboardManager.cpp
	Created: 19 Apr 2017 10:57:53pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

juce_ImplementSingleton(DashboardManager)

ApplicationProperties& getAppProperties();
String getAppVersion();

DashboardManager::DashboardManager() :
	BaseManager("Dashboards")
{
	editMode = addBoolParameter("Edit Mode", "If checked, items are editable. If not, items are normally usable", true);
	snapping = addBoolParameter("Snapping", "If checked, items are automatically aligned when dragging them closed to other ones", true);

	tabsBGColor = addColorParameter("Tabs BG Color", "Color for the tabs in the web view", NORMAL_COLOR);
	tabsLabelColor = addColorParameter("Tabs Label Color", "Color for the tabs in the web view", TEXT_COLOR);
	tabsBorderColor = addColorParameter("Tabs Border Color", "Color for the tabs in the web view", Colours::black);
	tabsBorderWidth = addFloatParameter("Tabs Border Width", "Width for the border of tabs in the web view", 0, 0);
	tabsSelectedBGColor = addColorParameter("Tabs Selected BG Color", "Color for the tabs in the web view", HIGHLIGHT_COLOR);
	tabsSelectedLabelColor = addColorParameter("Tabs Selected Label Color", "Color for the tabs in the web view", Colours::black);
	tabsSelectedBorderColor = addColorParameter("Tabs Selected Border Color", "Color for the tabs in the web view", Colours::black);
	tabsSelectedBorderWidth = addFloatParameter("Tabs Selected Border Width", "Width for the border of tabs in the web view", 0, 0);

#if ORGANICUI_USE_WEBSERVER
	serverRootPath = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile(OrganicApplication::getInstance()->getApplicationName() + "/dashboard");
#endif
}

DashboardManager::~DashboardManager()
{
#if ORGANICUI_USE_WEBSERVER
	if (server != nullptr)
	{
		server->stop();
		server.reset();
	}
#endif

#if ORGANICUI_USE_SERVUS
	servusThread.stopThread(1000);
#endif

	DashboardItemFactory::deleteInstance();

	Engine::mainEngine->removeEngineListener(this);
}

#if ORGANICUI_USE_WEBSERVER
void DashboardManager::setupServer()
{
	server.reset();

	if (Engine::mainEngine->isClearing) return;

	if (isCurrentlyLoadingData || Engine::mainEngine->isLoadingFile)
	{
		Engine::mainEngine->addEngineListener(this);
		return;
	}

	if (!ProjectSettings::getInstance()->enableServer->boolValue())
	{
		LOG("Dashboard server is not running");
#if ORGANICUI_USE_SERVUS
		servusThread.stopThread(1000);
#endif

		return;
	}

	int port = ProjectSettings::getInstance()->serverPort->intValue();

	/*
	File k = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("server.key");
	File c = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("server.crt");

	if (k.existsAsFile() && c.existsAsFile())
	{
		try
		{
			server.reset(new SecureWebSocketServer(c.getFullPathName(), k.getFullPathName()));
		}
		catch (std::exception e)
		{
			NLOGERROR(niceName, "Error creating secure server : " << e.what());
			return;
		}
	}
	else
	{
	*/
	server.reset(new SimpleWebSocketServer());
	//}


	server->handler = this;

	File f = Engine::mainEngine->getFile();
	File serverLocalPath;
	if (f.existsAsFile()) serverLocalPath = f.getParentDirectory().getChildFile("dashboard");
	if (serverLocalPath.exists() && serverLocalPath.isDirectory())
	{
		LOG("Found local dashboard in project's folder, using this one.");
		server->rootPath = serverLocalPath;
	}
	else server->rootPath = serverRootPath;

	server->addWebSocketListener(this);
	server->start(port);

#if ORGANICUI_USE_SERVUS
	servusThread.setupZeroconf();
#endif

	LOG("Dashboard server is running on port " << port << " on IPs :\n" + NetworkHelpers::getLocalIPs().joinIntoString("\n"));
}

void DashboardManager::connectionOpened(const String& id)
{
	LOG("New browser connection to the Dashboard from " << id);
	//if (server == nullptr) return;
	//String s = JSON::toString(data, true);
	//server->sendTo(s, id);

}

void DashboardManager::messageReceived(const String& id, const String& message)
{
	var data;
	Result result = JSON::parse(message, data);

	if (result.failed())
	{
		DBG("Error parsing: " << message << ", error : " << result.getErrorMessage());
		return;
	}



	if (data.hasProperty("setDashboard"))
	{
		if (Dashboard* d = getItemWithName(data.getProperty("setDashboard", ""), true))
		{
			bool sClients = data.getProperty("setInClients", true);
			bool sNative = data.getProperty("setInNative", true);
			setCurrentDashboard(d, sClients, sNative, id);
		}
		return;
	}

	String add = data.getProperty("controlAddress", "");
	if (add.isNotEmpty())
	{
		Controllable* c = Engine::mainEngine->getControllableForAddress(add);
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

var DashboardManager::getServerData()
{
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("dataType", "all");
	data.getDynamicObject()->setProperty("appName", OrganicApplication::getInstance()->getApplicationName());
	data.getDynamicObject()->setProperty("appVersion", OrganicApplication::getInstance()->getApplicationVersion());
	data.getDynamicObject()->setProperty("osName", SystemStats::getOperatingSystemName());
	data.getDynamicObject()->setProperty("osType", SystemStats::getOperatingSystemType());
	data.getDynamicObject()->setProperty("computerName", SystemStats::getComputerName());
	data.getDynamicObject()->setProperty("userName", SystemStats::getFullUserName());

	if (ProjectSettings::getInstance()->dashboardPassword != nullptr)
	{
		String pass = ProjectSettings::getInstance()->dashboardPassword->stringValue();
		if (pass.isNotEmpty())
		{
			data.getDynamicObject()->setProperty("password", pass);
			data.getDynamicObject()->setProperty("unlockOnce", ProjectSettings::getInstance()->unlockOnce->boolValue());
		}
	}

	var iData;
	for (auto& d : items)
	{
		iData.append(d->getServerData());
	}
	data.getDynamicObject()->setProperty("items", iData);
	return data;
}


bool DashboardManager::handleHTTPRequest(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
{
	String dataStr;
	SimpleWeb::CaseInsensitiveMultimap header;

	if (String(request->path) == "/data")
	{
		header.emplace("Content-Type", "application/json");
		var data = getServerData();

		var tabsData(new DynamicObject());
		tabsData.getDynamicObject()->setProperty("bgColor", tabsBGColor->value);
		tabsData.getDynamicObject()->setProperty("labelColor", tabsLabelColor->value);
		tabsData.getDynamicObject()->setProperty("borderColor", tabsBorderColor->value);
		tabsData.getDynamicObject()->setProperty("borderWidth", tabsBorderWidth->floatValue());
		tabsData.getDynamicObject()->setProperty("bgColorSelected", tabsSelectedBGColor->value);
		tabsData.getDynamicObject()->setProperty("labelColorSelected", tabsSelectedLabelColor->value);
		tabsData.getDynamicObject()->setProperty("borderColorSelected", tabsSelectedBorderColor->value);
		tabsData.getDynamicObject()->setProperty("borderWidthSelected", tabsSelectedBorderWidth->floatValue());
		data.getDynamicObject()->setProperty("tabs", tabsData);

		dataStr = JSON::toString(data, true);
	}
	else if (String(request->path) == "/fileData")
	{
		SimpleWeb::CaseInsensitiveMultimap query = request->parse_query_string();

		// If key not found in map iterator to end is returned
		auto arg = query.find("controlAddress");
		if (arg == query.end())
		{
			header.emplace("Content-Type", "text/html");
			dataStr = "Missing controlAddress argument in query";
		}
		else
		{
			if (FileParameter* fp = dynamic_cast<FileParameter*>(Engine::mainEngine->getControllableForAddress(arg->second)))
			{
				File f = fp->getFile();
				if (f.existsAsFile()) server->serveFile(f, response);
			}
			else
			{
				DBG(arg->second << " has not been found or is not a File Parameter");
				*response << "HTTP/1.1 404 Not Found";
			}

			return true;
		}
	}
	else if (customHandleHTTPRequestFunc)
	{
		dataStr = customHandleHTTPRequestFunc(response, request, header);
	}



	if (dataStr.isNotEmpty())
	{
		header.emplace("Content-Length", String(dataStr.length()).toStdString());
		header.emplace("Accept-range", "bytes");
		header.emplace("Access-Control-Allow-Origin", "*");

		response->write(SimpleWeb::StatusCode::success_ok, header);
		*response << dataStr;

		return true;
	}

	return false;
}

#if SIMPLEWEB_SECURE_SUPPORTED
bool DashboardManager::handleHTTPSRequest(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
{
	String dataStr;
	SimpleWeb::CaseInsensitiveMultimap header;

	if (String(request->path) == "/data")
	{
		var data = getServerData();

		var tabsData(new DynamicObject());
		tabsData.getDynamicObject()->setProperty("bgColor", tabsBGColor->value);
		tabsData.getDynamicObject()->setProperty("labelColor", tabsLabelColor->value);
		tabsData.getDynamicObject()->setProperty("borderColor", tabsBorderColor->value);
		tabsData.getDynamicObject()->setProperty("borderWidth", tabsBorderWidth->floatValue());
		tabsData.getDynamicObject()->setProperty("bgColorSelected", tabsSelectedBGColor->value);
		tabsData.getDynamicObject()->setProperty("labelColorSelected", tabsSelectedLabelColor->value);
		tabsData.getDynamicObject()->setProperty("borderColorSelected", tabsSelectedBorderColor->value);
		tabsData.getDynamicObject()->setProperty("borderWidthSelected", tabsSelectedBorderWidth->floatValue());
		data.getDynamicObject()->setProperty("tabs", tabsData);

		dataStr = JSON::toString(data, true);

		header.emplace("Content-Length", String(dataStr.length()).toStdString());
		header.emplace("Content-Type", "application/json");
		header.emplace("Accept-range", "bytes");
		header.emplace("Access-Control-Allow-Origin", "*");

		response->write(SimpleWeb::StatusCode::success_ok, header);
		*response << dataStr;

		return true;
	}
	else if (String(request->path) == "/fileData")
	{
		SimpleWeb::CaseInsensitiveMultimap query = request->parse_query_string();

		// If key not found in map iterator to end is returned
		auto arg = query.find("controlAddress");
		if (arg == query.end())
		{
			header.emplace("Content-Type", "text/html");
			dataStr = "Missing controlAddress argument in query";
		}
		else
		{
			if (FileParameter* fp = dynamic_cast<FileParameter*>(Engine::mainEngine->getControllableForAddress(arg->second)))
			{
				File f = fp->getFile();
				if (f.existsAsFile()) server->serveFile(f, response);
			}
			else
			{
				DBG(arg->second << " has not been found or is not a File Parameter");
				*response << "HTTP/1.1 404 Not Found";
			}

			return true;
		}
	}

	return false;
}
#endif

void DashboardManager::setupDownloadURL(const String& _downloadURL)
{
	downloadURL = _downloadURL;
	if (!serverRootPath.exists()) downloadDashboardFiles();
}

void DashboardManager::downloadDashboardFiles()
{
	if (downloadURL.isEmpty())
	{
		DBG("No download URL for dashboard, exiting");
		return;
	}

	LOG("Downloading dashboard files...");
	downloadedFileZip = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile(OrganicApplication::getInstance()->getApplicationName() + "/dashboard.zip");
	downloadTask = URL(downloadURL).downloadToFile(downloadedFileZip, URL::DownloadTaskOptions().withListener(this));

	if (downloadTask == nullptr)
	{
		LOGERROR("Error downloading dashboard files");
	}
}

void DashboardManager::progress(URL::DownloadTask* task, int64 bytesDownloaded, int64 bytesTotal)
{
}

void DashboardManager::finished(URL::DownloadTask* task, bool success)
{
	if (success) LOG("Dashboard downloaded. Extracting to " << serverRootPath.getFullPathName());
	else
	{
		LOGERROR("Dashboard download error");
		return;
	}

	if (serverRootPath.exists()) serverRootPath.deleteRecursively();

	ZipFile zf(downloadedFileZip);
	zf.uncompressTo(serverRootPath);
	downloadedFileZip.deleteFile();

	LOG("You got a new dashboard my friend !");// << zf.getNumEntries() << " files downloaded to " << serverRootPath.getFullPathName());
}
#endif //ORGANICUI_USE_WEBSERVER


void DashboardManager::addItemInternal(Dashboard* item, var data)
{
	item->addDashboardListener(this);
	askForRefresh(nullptr);
}

void DashboardManager::removeItemInternal(Dashboard* item)
{
	item->removeDashboardListener(this);
	askForRefresh(nullptr);
}

void DashboardManager::setCurrentDashboard(Dashboard* d, bool setInClients, bool setInNative, StringArray excludeIds)
{
	if (d == nullptr) return;

	if (setInNative)
	{
		if (DashboardManagerView* v = ShapeShifterManager::getInstance()->getContentForType<DashboardManagerView>())
		{
			MessageManager::getInstance()->callAsync([v, d]() {v->setCurrentDashboard(d); });
		}
	}

#if ORGANICUI_USE_WEBSERVER
	if (setInClients && server != nullptr && server->getNumActiveConnections() > 0)
	{
		var data(new DynamicObject());
		data.getDynamicObject()->setProperty("setDashboard", d->shortName);
		if (excludeIds.isEmpty()) server->send(JSON::toString(data));
		else server->sendExclude(JSON::toString(data), excludeIds);
	}
#endif
}

void DashboardManager::parameterFeedback(var data)
{
#if ORGANICUI_USE_WEBSERVER
	if (server != nullptr && server->getNumActiveConnections() > 0)
	{
		data.getDynamicObject()->setProperty("dataType", "feedback");
		server->send(JSON::toString(data));
	}
#endif
}

void DashboardManager::dashboardFeedback(var data)
{
#if ORGANICUI_USE_WEBSERVER
	if (server != nullptr && server->getNumActiveConnections() > 0)
	{
		data.getDynamicObject()->setProperty("dataType", "dashboardFeedback");
		server->send(JSON::toString(data));
	}
#endif
}


void DashboardManager::askForRefresh(Dashboard* d)
{
#if ORGANICUI_USE_WEBSERVER
	if (server != nullptr && server->getNumActiveConnections() > 0)
	{
		var data(new DynamicObject());
		data.getDynamicObject()->setProperty("refresh", d != nullptr ? d->shortName : "*");
		server->send(JSON::toString(data));
	}
#endif
}

void DashboardManager::endLoadFile()
{
	Engine::mainEngine->removeEngineListener(this);

#if ORGANICUI_USE_WEBSERVER
	setupServer();
#endif
}


#if ORGANICUI_USE_SERVUS
ServusThread::ServusThread() :
	Thread("Dashboard Zeroconf"),
	servus("_http._tcp")
{
}

void ServusThread::setupZeroconf()
{
	if (ProjectSettings::getInstanceWithoutCreating() == nullptr || ProjectSettings::getInstance()->serverPort == nullptr) return;

	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;
	if (!isThreadRunning()) startThread();
}


void ServusThread::run()
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

	LOG("Dashboard Zeroconf service created : " << nameToAdvertise << ":" << portToAdvertise);
}
#endif
