/*
 ==============================================================================

 AppVersion.cpp
 Created: 29 Jul 2026 03:36:00am
 Author:  Leon Victor

 ==============================================================================
 */

 using namespace juce;

 AppVersion::AppVersion(const String& versionStr)
	{
		StringArray split;
		split.addTokens(versionStr, ".", "\"");

		jassert(split.size() == 3);

		major = split[0].getIntValue();
		minor = split[1].getIntValue();

		const int channelStartIdx = split[2].indexOfAnyOf("abcdefghijklmnopqrstuvwxyz", 0, true);
		if (channelStartIdx != -1)
		{
			const String patchStr = split[2].substring(0, channelStartIdx);
			patch = patchStr.getIntValue();

			const int channelEndIdx = split[2].lastIndexOfAnyOf("abcdefghijklmnopqrstuvwxyz", true);
			channelName = split[2].substring(channelStartIdx, channelEndIdx + 1);

			const String channelVersionStr = split[2].substring(channelEndIdx + 1);
			channelVersion = channelVersionStr.getIntValue();
		}
		else
		{
			patch = split[2].getIntValue();
		}
	}

	bool AppVersion::operator==(const AppVersion& other) const 
	{ 
		return major == other.major && minor == other.minor && patch == other.patch && channelName == other.channelName && channelVersion == other.channelVersion;
	}

	bool AppVersion::operator<(const AppVersion& other) const
	{
		if (isCustom())
		{
			if (channelName != other.channelName)
			{
				return false;
			}

			if (major < other.major)
			{
				return true;
			}
			else if (major > other.major)
			{
				return false;
			}

			if (minor < other.minor)
			{
				return true;
			}
			else if (minor > other.minor)
			{
				return false;
			}

			if (patch < other.patch)
			{
				return true;
			}
			else if (patch > other.patch)
			{
				return false;
			}

			if (channelVersion < other.channelVersion)
			{
				return true;
			}
			else if (channelVersion > other.channelVersion)
			{
				return false;
			}

			return false;
		}
		else
		{
			if (major < other.major)
			{
				return true;
			}
			else if (major > other.major)
			{
				return false;
			}

			if (minor < other.minor)
			{
				return true;
			}
			else if (minor > other.minor)
			{
				return false;
			}

			if (patch < other.patch)
			{
				return true;
			}
			else if (patch > other.patch)
			{
				return false;
			}

			// All other fields equals:
			// if both are beta, compare channel ver
			// if only one is beta, the non beta one is >
			// if none are beta, they are equal
			if (isBeta())
			{
				if (!other.isBeta())
				{
					return true;
				}
				else
				{
					if (channelVersion < other.channelVersion)
					{
						return true;
					}
					else if (channelVersion > other.channelVersion)
					{
						return false;
					}

					return false;
				}
			}
			else
			{
				if (other.isBeta())
				{
					return false;
				}
				else
				{
					return false;
				}
			}
		}
	}

	bool AppVersion::operator<=(const AppVersion& other) const
	{
		return *this == other || *this < other;
	}

	String AppVersion::toString() const 
	{ 
		String res = String(major) + "." + String(minor) + "." + String(patch);
		if (!isRelease())
		{
			res = res + channelName + String(channelVersion);
		}
		return res;
	}