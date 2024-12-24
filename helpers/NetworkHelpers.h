#pragma once

#if JUCE_WINDOWS
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#endif

class NetworkHelpers
{
public:
	static juce::String getLocalIP();

	static bool isIPLocal(const juce::String& tip);
	static juce::StringArray getLocalIPs();
	static juce::String getLocalIPForRemoteIP(const juce::String& remoteIP, bool returnDefaultLocalIPIfNotFound = true);

};

class NetworkInterfaceParameter :
	public EnumParameter
{
public:
	NetworkInterfaceParameter();
	~NetworkInterfaceParameter();

	void fillOptions();

	juce::String getIP();
	void getInterfaceIpNameMap(juce::HashMap<juce::String, juce::String>& map);
};