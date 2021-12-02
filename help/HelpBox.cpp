#include "HelpBox.h"

juce_ImplementSingleton(HelpBox)

HelpBox::HelpBox() :
	Thread("helpThread")
{
}

HelpBox::~HelpBox()
{
	stopThread(1000);
}

void HelpBox::loadHelp()
{
	//load from internet if possible
	if (!helpURL.isEmpty() && GlobalSettings::getInstance()->updateHelpOnStartup->boolValue()) startThread();
	else loadLocalHelp();
}

void HelpBox::loadLocalHelp()
{
	File helpFile = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("help.json");
	if (!helpFile.existsAsFile()) return;
	std::unique_ptr<InputStream> is(helpFile.createInputStream());
	helpData = JSON::parse(*is);
}

void HelpBox::setOverData(String dataID)
{
	if (overDataID == dataID) return;
	overDataID = dataID;
	helpListeners.call(&HelpListener::helpContentChanged);
}

void HelpBox::clearOverData(String dataID)
{
	if (overDataID == dataID || dataID == "") setOverData("");
}

void HelpBox::setPersistentData(String dataID)
{
	if (persistentDataID == dataID) return;
	persistentDataID = dataID;

	if (overDataID.isNotEmpty()) return;
	helpListeners.call(&HelpListener::helpContentChanged);
}


void HelpBox::clearPersistentData(String dataID)
{
	if (persistentDataID == dataID) setPersistentData("");
}

const String HelpBox::getContentForID(String dataID)
{
	if (dataID.isEmpty()) return "";
	return helpData.getProperty(dataID, "No help for " + dataID + " right now, but you can help me and propose content for this help");
}

const String HelpBox::getCurrentData()
{
	return getContentForID(overDataID.isEmpty() ? persistentDataID : overDataID);
}

void HelpBox::run()
{
	//Load async
	StringPairArray responseHeaders;
	int statusCode = 0;
	URL languageHelpURL = URL(helpURL.toString(false) + "help_"+ GlobalSettings::getInstance()->helpLanguage->getValueData().toString() + ".json");
	
	std::unique_ptr<InputStream> stream(languageHelpURL.createInputStream(
		URL::InputStreamOptions(URL::ParameterHandling::inAddress)
		.withExtraHeaders("Cache-Control: no-cache")
		.withConnectionTimeoutMs(2000)
		.withResponseHeaders(&responseHeaders)
		.withStatusCode(&statusCode)
	));
	
#if JUCE_WINDOWS
	if (statusCode != 200)
	{
		LOGWARNING("Failed to retrieve online help : " << helpURL.toString(false) << ", loading local file");
		loadLocalHelp();
		return;
	}
#endif

	DBG("Loading help online :  " << helpURL.toString(false) << ", status code : " << statusCode);

	
	if (stream != nullptr)
	{
		String content = stream->readEntireStreamAsString();
		var data = JSON::parse(content);
		
		
		if (data.isObject())
		{
			helpData = data; 

			File helpFile = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("help.json");
			if (helpFile.existsAsFile()) helpFile.deleteFile();
			std::unique_ptr<OutputStream> os(helpFile.createOutputStream());
			if (os == nullptr)
			{
				NLOG("Help", "Error saving the help file " + helpFile.getFullPathName() + "\nMaybe it is read-only ?");
				return;
			}
			JSON::writeToStream(*os, data);
			os->flush();
		}
		else
		{
			LOGERROR("Error while loading online help, help file is not valid");
			loadLocalHelp();
		}

	}
	else
	{
		
		LOGERROR("Error while trying to access to the online help file");
		loadLocalHelp();
	}
}
