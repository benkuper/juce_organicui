#include "ScriptUtil.h"
/*
  ==============================================================================

	ScriptUtil.cpp
	Created: 21 Feb 2017 9:17:23am
	Author:  Ben

  ==============================================================================
*/


juce_ImplementSingleton(ScriptUtil)

ScriptUtil::ScriptUtil() :
	ScriptTarget("util", this)
{
	scriptObject.setMethod("getTime", ScriptUtil::getTime);
	scriptObject.setMethod("getTimestamp", ScriptUtil::getTimestamp);
	scriptObject.setMethod("getFloatFromBytes", ScriptUtil::getFloatFromBytes);
	scriptObject.setMethod("getInt32FromBytes", ScriptUtil::getInt32FromBytes);
	scriptObject.setMethod("getInt64FromBytes", ScriptUtil::getInt32FromBytes);
	scriptObject.setMethod("getObjectProperties", ScriptUtil::getObjectPropertiesNames);

	scriptObject.setMethod("getIPs", ScriptUtil::getIPs);
	scriptObject.setMethod("encodeHMAC_SHA1", ScriptUtil::encodeHMAC_SHA1);
	scriptObject.setMethod("toBase64", ScriptUtil::toBase64);

	scriptObject.setMethod("readFile", ScriptUtil::readFileFromScript);
	scriptObject.setMethod("writeFile", ScriptUtil::writeFileFromScript);
	scriptObject.setMethod("createDirectory", ScriptUtil::createDirectoryFromScript);
	scriptObject.setMethod("launchFile", ScriptUtil::launchFileFromScript);
	scriptObject.setMethod("killApp", ScriptUtil::killAppFromScript);
}

var ScriptUtil::getTime(const var::NativeFunctionArgs &)
{
	return (float)(Time::getMillisecondCounter() / 1000.);
}

var ScriptUtil::getTimestamp(const var::NativeFunctionArgs&)
{
	return Time::currentTimeMillis()/1000;
}

var ScriptUtil::getFloatFromBytes(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 4) return 0;
	uint8_t bytes[4];
	for (int i = 0; i < 4; i++) bytes[i] = (uint8_t)(int)a.arguments[i];
	float result;
	memcpy(&result, &bytes, 4);
	return result;
}

var ScriptUtil::getInt32FromBytes(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 4) return 0;
	uint8_t bytes[4];
	for (int i = 0; i < 4; i++) bytes[i] = (uint8_t)(int)a.arguments[i];
	int result;
	memcpy(&result, &bytes, 4);
	return result;
}


var ScriptUtil::getInt64FromBytes(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 8) return 0;
	uint8_t bytes[8];
	for (int i = 0; i < 8; i++) bytes[i] = (uint8_t)(int)a.arguments[i];
	int64 result;
	memcpy(&result, &bytes, 8);
	return result;
}

var ScriptUtil::getObjectPropertiesNames(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0 || !a.arguments[0].isObject()) return var();

	NamedValueSet props = a.arguments[0].getDynamicObject()->getProperties();
	var result;
	for (auto& p : props)
	{
		result.append(p.name.toString());
	}

	return result;
}

/*
var ScriptUtil::cosFromScript(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 1) return 0;
	return cosf((float)a.arguments[0]);
}


var ScriptUtil::sinFromScript(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 1) return 0;
	return sinf((float)a.arguments[0]);
}


var ScriptUtil::atan2FromScript(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 2) return 0;
	return atan2((double)a.arguments[0], (double)a.arguments[1]);
}

var ScriptUtil::toDegrees(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 1) return 0;
	return radiansToDegrees((double)a.arguments[0]);
}

var ScriptUtil::toRadians(const var::NativeFunctionArgs & a)
{
	if (a.numArguments < 1) return 0;
	return degreesToRadians((double)a.arguments[0]);
}
*/

var ScriptUtil::getIPs(const var::NativeFunctionArgs& a)
{
	var result;

	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);
	Array<String> ips;
	for (auto& add : ad) ips.add(add.toString());
	ips.sort();
	for (auto& ip : ips) result.append(ip);

	return result;
}

var ScriptUtil::encodeHMAC_SHA1(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 2) return 0;

	MemoryBlock b = HMAC_SHA1::encode(a.arguments[0].toString(), a.arguments[1].toString());

	DBG("Encoding...\n" + a.arguments[0].toString() + "\n" + a.arguments[1].toString());

	uint8_t * data = (uint8_t *)b.getData();
	String dbgHex = "";
	for (int i = 0; i < b.getSize(); i++)
	{
		dbgHex += String::toHexString(data[i]) + " ";
	}
	DBG("DBG HEX : " << dbgHex);
	return Base64::toBase64(b.getData(), b.getSize());
}

var ScriptUtil::toBase64(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return 0;
	return Base64::toBase64(a.arguments[0].toString());
}

var ScriptUtil::readFileFromScript(const var::NativeFunctionArgs& args)
{
	String path = args.arguments[0].toString();

	if (!File::isAbsolutePath(path)) path = Engine::mainEngine->getFile().getParentDirectory().getChildFile(path).getFullPathName();

	File f(path);

	if (!f.existsAsFile()) return var();

	if (args.numArguments >= 2 && (int)args.arguments[1])
	{
		return JSON::parse(f);
	}
	else
	{
		FileInputStream fs(f);
		return fs.readEntireStreamAsString();
	}
}

var ScriptUtil::writeFileFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments < 2) return false;

	String path = args.arguments[0].toString();

	if (!File::isAbsolutePath(path)) path = Engine::mainEngine->getFile().getParentDirectory().getChildFile(path).getFullPathName();

	File f(path);

	bool overwriteIfExists = args.numArguments > 2 ? ((int)args.arguments[2] > 0) : false;
	if (f.existsAsFile())
	{
		if (overwriteIfExists) f.deleteFile();
		else
		{
			LOG("File already exists : " << f.getFileName() << ", you need to enable overwrite to replace its content.");
			return false;
		}
	}

	FileOutputStream fs(f);
	if (args.arguments[1].isObject())
	{
		JSON::writeToStream(fs, args.arguments[1]);
		return true;
	}

	return fs.writeText(args.arguments[1].toString(), false, false, "\n");
}

var ScriptUtil::createDirectoryFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return false;

	String path = args.arguments[0].toString();

	if (!File::isAbsolutePath(path)) path = Engine::mainEngine->getFile().getParentDirectory().getChildFile(path).getFullPathName();

	File f(path);

	if (f.exists())
	{
		LOG("Directory or file already exists : " << f.getFileName());
		return false;
	}
	else {
		f.createDirectory();
		return true;
	}
}

var ScriptUtil::launchFileFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return false;

	String path = args.arguments[0].toString();

	if (!File::isAbsolutePath(path)) path = Engine::mainEngine->getFile().getParentDirectory().getChildFile(path).getFullPathName();

	File f(path);

	if (f.existsAsFile())
	{
		return f.startAsProcess(args.numArguments > 1 ? args.arguments[1].toString() : "");
	}

	return false;
}

var ScriptUtil::killAppFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return false;
	String appName = args.arguments[0].toString();
	bool hardKill = args.numArguments > 1 ? (bool)args.arguments[1] : false;
#if JUCE_WINDOWS
	int result = system(String("taskkill " + String(hardKill ? "/f " : "") + "/im \"" + appName+ "\"").getCharPointer());
	if (result != 0) LOGWARNING("Problem killing app " + appName);
#else
	int result = system(String("killall " + String(hardKill ? "-9" : "-2") + " \"" + appName + "\"").getCharPointer());
	if (result != 0) LOGWARNING("Problem killing app " + appName);
#endif
	return var();
}
