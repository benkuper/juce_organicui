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
			if (ip.startsWith("192.168.4.") || ip.startsWith("192.168.43.")) return ip;
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
			if (ip.toString().startsWith("192.168.0.") || ip.toString().startsWith("192.168.1.") || ip.toString().startsWith("192.168.4.") || ip.toString().startsWith("192.168.43."))
			{
				result.add(ip.toString());
			}
		}
		return result;
	}
};
