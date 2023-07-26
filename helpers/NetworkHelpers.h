#pragma once

class NetworkHelpers
{
public:
	static juce::String getLocalIP()
	{
		juce::StringArray ips = getLocalIPs();
		for (auto &ip : ips)
		{
			if (ip.startsWith("192.168.0.") || ip.startsWith("192.168.1.") || ip.startsWith("192.168.2.") || ip.startsWith("192.168.3.")) return ip;
		}

		for (auto& ip : ips)
		{
			if (ip.startsWith("192.168")) return ip;
		}

		return ips.size() > 0 ? ips[0] : "";
	}

	static bool isIPLocal(const juce::String& tip)
	{
		juce::StringArray result;
		juce::Array<juce::IPAddress> ad;
		juce::IPAddress::findAllAddresses(ad);
		for (auto& ip : ad) if (ip.toString() == tip) return true;
		return false;
	}

	static juce::StringArray getLocalIPs()
	{
		juce::StringArray result;
		juce::Array<juce::IPAddress> ad;
		juce::IPAddress::findAllAddresses(ad);
		for (auto &ip : ad)
		{
			if (ip.toString().startsWith("192.168"))//.0.") || ip.toString().startsWith("192.168.1.") || ip.toString().startsWith("192.168.2.") || ip.toString().startsWith("192.168.3.") || ip.toString().startsWith("192.168.4.") || ip.toString().startsWith("192.168.43."))
			{
				result.add(ip.toString());
			}
		}
		return result;
	}

	static juce::String getLocalIPForRemoteIP(const juce::String &remoteIP, bool returnDefaultLocalIPIfNotFound = true)
	{
		if (remoteIP == "127.0.0.1") return "127.0.0.1";

		juce::StringArray ips;
		juce::Array<juce::IPAddress> ad;
		juce::IPAddress::findAllAddresses(ad);
		for (auto& ip : ad) ips.add(ip.toString());

		juce::StringArray ripSplit;
		ripSplit.addTokens(remoteIP, ".", "\"");
		for (auto& ip : ips)
		{
			juce::StringArray ipSplit;
			ipSplit.addTokens(ip, ".", "\"");
			if (ripSplit[0] == ipSplit[0] && ripSplit[1] == ipSplit[1] && ripSplit[2] == ipSplit[2]) return ip;
		}

		for (auto& ip : ips)
		{
			juce::StringArray ipSplit;
			ipSplit.addTokens(ip, ".", "\"");
			if (ripSplit[0] == ipSplit[0] && ripSplit[1] == ipSplit[1]) return ip;
		}

		for (auto& ip : ips)
		{
			juce::StringArray ipSplit;
			ipSplit.addTokens(ip, ".", "\"");
			if (ripSplit[0] == ipSplit[0]) return ip;
		}

		return returnDefaultLocalIPIfNotFound?getLocalIP():"";
	}
};
