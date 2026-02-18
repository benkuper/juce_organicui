#pragma once

#if JUCE_WINDOWS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "NetworkHelpers.h"
#endif

String NetworkHelpers::getLocalIP()
{
	StringArray ips = getLocalIPs();
	for (auto& ip : ips)
	{
		if (ip.startsWith("192.168.0.") || ip.startsWith("192.168.1.") || ip.startsWith("192.168.2.") || ip.startsWith("192.168.3.")) return ip;
	}

	for (auto& ip : ips)
	{
		if (ip.startsWith("192.168")) return ip;
	}

	return ips.size() > 0 ? ips[0] : "";
}

bool NetworkHelpers::isIPLocal(const String& tip)
{
	StringArray result;
	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);
	for (auto& ip : ad) if (ip.toString() == tip) return true;
	return false;
}

StringArray NetworkHelpers::getLocalIPs()
{
	StringArray result;
	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);
	for (auto& ip : ad)
	{
		if (ip.toString().startsWith("192.168"))//.0.") || ip.toString().startsWith("192.168.1.") || ip.toString().startsWith("192.168.2.") || ip.toString().startsWith("192.168.3.") || ip.toString().startsWith("192.168.4.") || ip.toString().startsWith("192.168.43."))
		{
			result.add(ip.toString());
		}
	}
	return result;
}

String NetworkHelpers::getLocalIPForRemoteIP(const String& remoteIP, bool returnDefaultLocalIPIfNotFound)
{
	if (remoteIP == "127.0.0.1") return "127.0.0.1";

	StringArray ips;
	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);
	for (auto& ip : ad) ips.add(ip.toString());

	StringArray ripSplit;
	ripSplit.addTokens(remoteIP, ".", "\"");
	for (auto& ip : ips)
	{
		StringArray ipSplit;
		ipSplit.addTokens(ip, ".", "\"");
		if (ripSplit[0] == ipSplit[0] && ripSplit[1] == ipSplit[1] && ripSplit[2] == ipSplit[2]) return ip;
	}

	for (auto& ip : ips)
	{
		StringArray ipSplit;
		ipSplit.addTokens(ip, ".", "\"");
		if (ripSplit[0] == ipSplit[0] && ripSplit[1] == ipSplit[1]) return ip;
	}

	for (auto& ip : ips)
	{
		StringArray ipSplit;
		ipSplit.addTokens(ip, ".", "\"");
		if (ripSplit[0] == ipSplit[0]) return ip;
	}

	return returnDefaultLocalIPIfNotFound ? getLocalIP() : "";
}

NetworkInterfaceParameter::NetworkInterfaceParameter() :
	EnumParameter("Network Interface", "Specify on which interface you want to send or receive stuff. Choose Auto to have automatic detection.")
{
	fillOptions();
}

NetworkInterfaceParameter::~NetworkInterfaceParameter()
{
}

void NetworkInterfaceParameter::fillOptions()
{
	clearOptions();
	addOption("Auto", String());

	HashMap<String, String> ipNameMap;
	getInterfaceIpNameMap(ipNameMap);

	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);

	StringArray noNameIPs;
	for (auto& ip : ad)
	{
		if (ipNameMap.contains(ip.toString()))
			addOption(ipNameMap[ip.toString()] + " (" + ip.toString() + ")", ip.toString());
		else
			noNameIPs.add(ip.toString());
	}

	for (auto& ip : noNameIPs) addOption(ip, ip);
}

String NetworkInterfaceParameter::getIP()
{
	return getValueData().toString();
}


void NetworkInterfaceParameter::getInterfaceIpNameMap(HashMap<String, String>& map)
{
#if JUCE_WINDOWS
	ULONG bufferSize = 0;
	GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, nullptr, &bufferSize);

	std::vector<char> buffer(bufferSize);
	IP_ADAPTER_ADDRESSES* adapterAddresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());

	if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &bufferSize) == NO_ERROR)
	{
		for (IP_ADAPTER_ADDRESSES* adapter = adapterAddresses; adapter != nullptr; adapter = adapter->Next)
		{
			//std::wcout << L"Interface: " << adapter->FriendlyName << std::endl;

			for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next)
			{
				SOCKADDR* addr = unicast->Address.lpSockaddr;
				char ip[INET6_ADDRSTRLEN] = { 0 };

				if (addr->sa_family == AF_INET)
					inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr, ip, sizeof(ip));
				else if (addr->sa_family == AF_INET6)
					inet_ntop(AF_INET6, &((struct sockaddr_in6*)addr)->sin6_addr, ip, sizeof(ip));

				if (ip[0] != '\0')
				{
					map.set(String(ip), adapter->FriendlyName);
				}
			}
		}
	}
#else
	struct ifaddrs* ifAddrStruct = nullptr;
	getifaddrs(&ifAddrStruct);

	for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == nullptr)
			continue;

		// Check for IPv4 or IPv6
		if (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)
		{
			void* addr = nullptr;
			if (ifa->ifa_addr->sa_family == AF_INET)
				addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
			else
				addr = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;

			char ip[INET6_ADDRSTRLEN];
			inet_ntop(ifa->ifa_addr->sa_family, addr, ip, sizeof(ip));

			map.set(String(ip), String(ifa->ifa_name));
		}
	}

	if (ifAddrStruct != nullptr)
		freeifaddrs(ifAddrStruct);
#endif
}
