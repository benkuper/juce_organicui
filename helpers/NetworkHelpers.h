#pragma once

class NetworkHelpers
{
public:
	static String getLocalIP()
	{
		StringArray ips = getLocalIPs();
		for (auto &ip : ips)
		{
			if (ip.startsWith("192.168.0") || ip.startsWith("192.168.1") || ip.startsWith("192.168.2") || ip.startsWith("192.168.3")) return ip;
		}

		for (auto& ip : ips)
		{
			if (ip.startsWith("192.168")) return ip;
		}

		return ips.size() > 0 ? ips[0] : "";
	}

	static bool isIPLocal(const String& tip)
	{
		StringArray result;
		Array<IPAddress> ad;
		IPAddress::findAllAddresses(ad);
		for (auto& ip : ad) if (ip.toString() == tip) return true;
		return false;
	}

	static StringArray getLocalIPs()
	{
		StringArray result;
		Array<IPAddress> ad;
		IPAddress::findAllAddresses(ad);
		for (auto &ip : ad)
		{
			if (ip.toString().startsWith("192.168"))//.0.") || ip.toString().startsWith("192.168.1.") || ip.toString().startsWith("192.168.2.") || ip.toString().startsWith("192.168.3.") || ip.toString().startsWith("192.168.4.") || ip.toString().startsWith("192.168.43."))
			{
				result.add(ip.toString());
			}
		}
		return result;
	}

	static String getLocalIPForRemoteIP(const String &remoteIP, bool returnDefaultLocalIPIfNotFound = true)
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

		return returnDefaultLocalIPIfNotFound?getLocalIP():"";
	}
};
