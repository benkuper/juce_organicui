#pragma once

class NetworkHelpers
{
public:
	static String getLocalIP()
	{
		StringArray ips = getLocalIPs();
		for (auto &ip : ips)
		{
			if (ip.startsWith("192.168.0.") || ip.startsWith("192.168.1.") || ip.startsWith("192.168.2.")) return ip;
			if (ip.startsWith("192.168.3.") || ip.startsWith("192.168.4.") || ip.startsWith("192.168.43.")) return ip;
			if (ip.startsWith("192.168")) return ip;
		}
        
        return "";	
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
		StringArray ips = getLocalIPs();
		StringArray ripSplit;
		ripSplit.addTokens(remoteIP, ".", "\"");
		for (auto& ip : ips)
		{
			StringArray ipSplit;
			ipSplit.addTokens(ip, ".", "\"");
			if (ripSplit[0] == ipSplit[0] && ripSplit[1] == ipSplit[1] && ripSplit[2] == ipSplit[2]) return ip;
		}

		return returnDefaultLocalIPIfNotFound?getLocalIP():"";
	}
};
