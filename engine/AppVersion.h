/*
 ==============================================================================

 AppVersion.h
 Created: 29 Jul 2026 03:36:00am
 Author:  Leon Victor

 ==============================================================================
 */

#pragma once

class AppVersion
{
public:
	AppVersion(const juce::String& versionStr);

	bool operator==(const AppVersion& other) const;
	bool operator<(const AppVersion& other) const;
	bool operator<=(const AppVersion& other) const;

	bool isBeta() const { return channelName == "b"; }
	bool isRelease() const { return channelName.isEmpty(); }
	bool isCustom() const { return !isBeta() && !isRelease(); }

	int getMajor() const { return major; }
	int getMinor() const { return minor; }
	int getPatch() const { return patch; }

	juce::String getChannel() const { return channelName; }
	int getChannelVersion() const { return channelVersion; }

	juce::String toString() const;

private:
	int major = 0;
	int minor = 0;
	int patch = 0;
	juce::String channelName = "";
	int channelVersion = -1;
};

 