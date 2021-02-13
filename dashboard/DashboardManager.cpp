#include "DashboardManager.h"
/*
  ==============================================================================

    DashboardManager.cpp
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(DashboardManager)

ApplicationProperties& getAppProperties();
String getAppVersion();

DashboardManager::DashboardManager() :
	BaseManager("Dashboards")
{
	editMode = addBoolParameter("Edit Mode", "If checked, items are editable. If not, items are normally usable", true);
	snapping = addBoolParameter("Snapping", "If checked, items are automatically aligned when dragging them closed to other ones", true);

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

	server.reset(new SimpleWebSocketServer());
	server->rootPath = serverRootPath;
	server->addWebSocketListener(this);
	server->start(port);

#if ORGANICUI_USE_SERVUS
	servusThread.setupZeroconf();
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

void DashboardManager::setupDownloadURL(const String& _downloadURL)
{
	downloadURL = URL(_downloadURL);
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
	downloadedFileZip = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile(OrganicApplication::getInstance()->getApplicationName()+"/dashboard.zip");
	downloadTask = downloadURL.downloadToFile(downloadedFileZip, "", this);
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
	LOG("Dashboard download success ? " << (int)success);
	ZipFile zf(downloadedFileZip);
	zf.uncompressTo(serverRootPath);
	LOG("Dashboard : " << zf.getNumEntries() << " files downloaded to " << serverRootPath.getFullPathName());
	downloadedFileZip.deleteFile();
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
	if(ProjectSettings::getInstanceWithoutCreating() == nullptr || ProjectSettings::getInstance()->serverPort == nullptr) return;

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
