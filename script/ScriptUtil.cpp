#include "ScriptUtil.h"
/*
  ==============================================================================

	ScriptUtil.cpp
	Created: 21 Feb 2017 9:17:23am
	Author:  Ben

  ==============================================================================
*/


juce_ImplementSingleton(ScriptUtil)

String getAppVersion();

ScriptUtil::ScriptUtil() :
	ScriptTarget("util", this)
{
	scriptObject.setMethod("getTime", ScriptUtil::getTime);
	scriptObject.setMethod("getTimestamp", ScriptUtil::getTimestamp);
	scriptObject.setMethod("getFloatFromBytes", ScriptUtil::getFloatFromBytes);
	scriptObject.setMethod("getInt32FromBytes", ScriptUtil::getInt32FromBytes);
	scriptObject.setMethod("getInt64FromBytes", ScriptUtil::getInt32FromBytes);
	scriptObject.setMethod("getObjectProperties", ScriptUtil::getObjectProperties);
	scriptObject.setMethod("getObjectMethods", ScriptUtil::getObjectMethods);

	scriptObject.setMethod("getIPs", ScriptUtil::getIPs);
	scriptObject.setMethod("encodeHMAC_SHA1", ScriptUtil::encodeHMAC_SHA1);
	scriptObject.setMethod("toBase64", ScriptUtil::toBase64);
	scriptObject.setMethod("fromBase64", ScriptUtil::fromBase64);

	scriptObject.setMethod("fileExists", ScriptUtil::fileExistsFromScript);
	scriptObject.setMethod("readFile", ScriptUtil::readFileFromScript);
	scriptObject.setMethod("writeFile", ScriptUtil::writeFileFromScript);
	scriptObject.setMethod("directoryExists", ScriptUtil::directoryExistsFromScript);
	scriptObject.setMethod("createDirectory", ScriptUtil::createDirectoryFromScript);
	scriptObject.setMethod("launchFile", ScriptUtil::launchFileFromScript);
	scriptObject.setMethod("killApp", ScriptUtil::killAppFromScript);
	scriptObject.setMethod("getOSInfos", ScriptUtil::getOSInfosFromScript);
	scriptObject.setMethod("getAppVersion", ScriptUtil::getAppVersionFromScript);
	scriptObject.setMethod("getEnvironmentVariable", ScriptUtil::getEnvironmentVariableFromScript);

	scriptObject.setMethod("copyToClipboard", ScriptUtil::copyToClipboardFromScript);
	scriptObject.setMethod("getFromClipboard", ScriptUtil::getFromClipboardFromScript);

	scriptObject.setMethod("showMessageBox", ScriptUtil::showMessageBox);
	scriptObject.setMethod("showOkCancelBox", ScriptUtil::showOkCancelBox);
	scriptObject.setMethod("showYesNoCancelBox", ScriptUtil::showYesNoCancelBox);
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
	for (int i = 0; i < 4; ++i) bytes[i] = (uint8_t)(int)a.arguments[i];
	float result;
	memcpy(&result, &bytes, 4);
	return result;
}

var ScriptUtil::getInt32FromBytes(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 4) return 0;
	uint8_t bytes[4];
	for (int i = 0; i < 4; ++i) bytes[i] = (uint8_t)(int)a.arguments[i];
	int result;
	memcpy(&result, &bytes, 4);
	return result;
}


var ScriptUtil::getInt64FromBytes(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 8) return 0;
	uint8_t bytes[8];
	for (int i = 0; i < 8; ++i) bytes[i] = (uint8_t)(int)a.arguments[i];
	int64 result;
	memcpy(&result, &bytes, 8);
	return result;
}

var ScriptUtil::getObjectMethods(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0 || !a.arguments[0].isObject()) return var();

	NamedValueSet props = a.arguments[0].getDynamicObject()->getProperties();
	var result;

	for (auto& p : props)
	{
		if (p.value.isMethod())
		{
			result.append(p.name.toString());
		}
	}

	return result;
}


var ScriptUtil::getObjectProperties(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0 || !a.arguments[0].isObject()) return var();

	NamedValueSet props = a.arguments[0].getDynamicObject()->getProperties();
	var result;

	bool includeObjects = (a.numArguments > 1) ? (bool)a.arguments[1] : true;
	bool includeParameters = (a.numArguments > 2) ? (bool)a.arguments[2] : true;

	for (auto& p : props)
	{
		if ((includeObjects && p.value.isObject()) ||
			(includeParameters && !p.value.isObject() && !p.value.isMethod()))
		{
			result.append(p.name.toString());
		}
	}

	return result;
}

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

	uint8_t* data = (uint8_t*)b.getData();
	String dbgHex = "";
	for (int i = 0; i < b.getSize(); ++i)
	{
		dbgHex += String::toHexString(data[i]) + " ";
	}
	return Base64::toBase64(b.getData(), b.getSize());
}

var ScriptUtil::toBase64(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return 0;
	std::string result = base64_encode((const unsigned char *)a.arguments[0].toString().toStdString().c_str(), a.arguments[0].toString().length());
	return String(result);
}

var ScriptUtil::fromBase64(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return 0;

	var result;
	String s = base64_decode((const std::string)a.arguments[0].toString().toStdString());
	CharPointer_UTF8 cp = s.getCharPointer();
	for (int i = 0; i < s.length(); i++) result.append((uint8_t)cp[i]);
	return result;
}

var ScriptUtil::fileExistsFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return false;
	return File(args.arguments[0]).existsAsFile();
}

var ScriptUtil::readFileFromScript(const var::NativeFunctionArgs& args)
{
	String path = args.arguments[0].toString();

	if (!File::isAbsolutePath(path))
	{
		File folder = Engine::mainEngine->getFile().getParentDirectory();
		if (args.numArguments >= 3 && (int)args.arguments[2])
		{
			String scriptPath = args.thisObject.getProperty("scriptPath", "").toString();
			folder = File(scriptPath).getParentDirectory();
		}

		path = folder.getChildFile(path).getFullPathName();
	}

	File f(path);


	if (!f.existsAsFile())
	{
		LOGWARNING("File not found : " << f.getFullPathName());
		return var();
	}

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

	if (!File::isAbsolutePath(path))
	{
		File folder = Engine::mainEngine->getFile().getParentDirectory();
		if (args.numArguments >= 3 && (int)args.arguments[2])
		{
			String scriptPath = args.thisObject.getProperty("scriptPath", "").toString();
			folder = File(scriptPath).getParentDirectory();
		}

		path = folder.getChildFile(path).getFullPathName();
	}


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

var ScriptUtil::directoryExistsFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return false;
	File f(args.arguments[0]);
	return f.exists() && f.isDirectory();
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
	int result = system(String("taskkill " + String(hardKill ? "/f " : "") + "/im \"" + appName + "\"").getCharPointer());
	if (result != 0) LOGWARNING("Problem killing app " + appName);
#else
	int result = system(String("killall " + String(hardKill ? "-9" : "-2") + " \"" + appName + "\"").getCharPointer());
	if (result != 0) LOGWARNING("Problem killing app " + appName);
#endif
	return var();
}

var ScriptUtil::getOSInfosFromScript(const var::NativeFunctionArgs&)
{
	DynamicObject* result = new DynamicObject();
	result->setProperty("name", SystemStats::getOperatingSystemName());
	result->setProperty("type", SystemStats::getOperatingSystemType());
	result->setProperty("computerName", SystemStats::getComputerName());
	result->setProperty("language", SystemStats::getUserLanguage());
	result->setProperty("username", SystemStats::getFullUserName());

	return var(result);
}

var ScriptUtil::getAppVersionFromScript(const var::NativeFunctionArgs&)
{
	return getAppVersion();
}

var ScriptUtil::getEnvironmentVariableFromScript(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return var();
	return SystemStats::getEnvironmentVariable(a.arguments[0], "");
}

var ScriptUtil::copyToClipboardFromScript(const var::NativeFunctionArgs& args)
{
	String s = "";
	for (int i = 0; i < args.numArguments; ++i)
	{
		s += (i > 0 ? " " : "") + args.arguments[i].toString();
	}
	SystemClipboard::copyTextToClipboard(s);
	return s;
}

var ScriptUtil::getFromClipboardFromScript(const var::NativeFunctionArgs& args)
{
	return SystemClipboard::getTextFromClipboard();
}

var ScriptUtil::showMessageBox(const var::NativeFunctionArgs& args)
{
	if (args.numArguments < 2) return false;

	AlertWindow::AlertIconType iconType = AlertWindow::NoIcon;
	if (args.numArguments >= 3)
	{
		String s = args.arguments[2].toString();
		if (s == "warning") iconType = AlertWindow::WarningIcon;
		if (s == "info") iconType = AlertWindow::AlertIconType::InfoIcon;
		if (s == "question") iconType = AlertWindow::AlertIconType::QuestionIcon;
	}

	String title = args.arguments[0].toString();
	String message = args.arguments[1].toString();
	String buttonText = args.numArguments >= 4 ? args.arguments[3].toString() : "";
	AlertWindow::showMessageBox(iconType, title, message, buttonText);

	return var();
}

var ScriptUtil::showOkCancelBox(const var::NativeFunctionArgs& args)
{
	if (args.numArguments < 2) return false;

	AlertWindow::AlertIconType iconType = AlertWindow::NoIcon;
	if (args.numArguments >= 3)
	{
		String s = args.arguments[2].toString();
		if (s == "warning") iconType = AlertWindow::WarningIcon;
		if (s == "info") iconType = AlertWindow::AlertIconType::InfoIcon;
		if (s == "question") iconType = AlertWindow::AlertIconType::QuestionIcon;
	}

	String title = args.arguments[0].toString();
	String message = args.arguments[1].toString();
	String button1Text = args.numArguments >= 4 ? args.arguments[3].toString() : "";
	String button2Text = args.numArguments >= 5 ? args.arguments[4].toString() : "";
	bool result = AlertWindow::showOkCancelBox(iconType, title, message, button1Text,button2Text);

	return result;
}

var ScriptUtil::showYesNoCancelBox(const var::NativeFunctionArgs& args)
{
	if (args.numArguments < 2) return false;

	AlertWindow::AlertIconType iconType = AlertWindow::NoIcon;
	if (args.numArguments >= 3)
	{
		String s = args.arguments[2].toString();
		if (s == "warning") iconType = AlertWindow::WarningIcon;
		if (s == "info") iconType = AlertWindow::AlertIconType::InfoIcon;
		if (s == "question") iconType = AlertWindow::AlertIconType::QuestionIcon;
	}

	String title = args.arguments[0].toString();
	String message = args.arguments[1].toString();
	String button1Text = args.numArguments >= 4 ? args.arguments[3].toString() : "";
	String button2Text = args.numArguments >= 5 ? args.arguments[4].toString() : "";
	String button3Text = args.numArguments >= 6 ? args.arguments[5].toString() : "";
	int result = AlertWindow::showYesNoCancelBox(iconType, title, message, button1Text, button2Text, button3Text);

	return result;
}

inline std::string ScriptUtil::base64_encode(unsigned char const* src, unsigned int len) {
	const unsigned char base64_table[65] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	unsigned char* out, * pos;
	const unsigned char* end, * in;

	size_t olen;

	olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

	if (olen < len)
		return std::string(); /* integer overflow */

	std::string outStr;
	outStr.resize(olen);
	out = (unsigned char*)&outStr[0];

	end = src + len;
	in = src;
	pos = out;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		}
		else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
				(in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
	}

	return outStr;

}

inline std::string ScriptUtil::base64_decode(std::string const& data)
{
	const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
		7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
		0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

	unsigned char* p = (unsigned char*)data.c_str();
	int len = data.size();
	int pad = len > 0 && (len % 4 || p[len - 1] == '=');
	const size_t L = ((len + 3) / 4 - pad) * 4;
	std::string str(L / 4 * 3 + pad, '\0');

	for (size_t i = 0, j = 0; i < L; i += 4)
	{
		int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
		str[j++] = n >> 16;
		str[j++] = n >> 8 & 0xFF;
		str[j++] = n & 0xFF;
	}
	if (pad)
	{
		int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
		str[str.size() - 1] = n >> 16;

		if (len > L + 2 && p[L + 2] != '=')
		{
			n |= B64index[p[L + 2]] << 6;
			str.push_back(n >> 8 & 0xFF);
		}
	}
	return str;
}
