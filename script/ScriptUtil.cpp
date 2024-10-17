/*
  ==============================================================================

	ScriptUtil.cpp
	Created: 21 Feb 2017 9:17:23am
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"
#include "ScriptUtil.h"

juce_ImplementSingleton(ScriptUtil)

String getAppVersion();

ScriptUtil::ScriptUtil() :
	ScriptTarget("util", this)
{
	scriptObject.getDynamicObject()->setMethod("getTime", ScriptUtil::getTime);
	scriptObject.getDynamicObject()->setMethod("getTimestamp", ScriptUtil::getTimestamp);
	scriptObject.getDynamicObject()->setMethod("delayThreadMS", ScriptUtil::delayThreadMS);

	scriptObject.getDynamicObject()->setMethod("getFloatFromBytes", ScriptUtil::getFloatFromBytes);
	scriptObject.getDynamicObject()->setMethod("getDoubleFromBytes", ScriptUtil::getDoubleFromBytes);
	scriptObject.getDynamicObject()->setMethod("floatToHexSeq", ScriptUtil::floatToHexSeq);
	scriptObject.getDynamicObject()->setMethod("getInt32FromBytes", ScriptUtil::getInt32FromBytes);
	scriptObject.getDynamicObject()->setMethod("getInt64FromBytes", ScriptUtil::getInt32FromBytes);
	scriptObject.getDynamicObject()->setMethod("doubleToHexSeq", ScriptUtil::doubleToHexSeq);
	scriptObject.getDynamicObject()->setMethod("hexStringToInt", ScriptUtil::hexStringToInt);
	scriptObject.getDynamicObject()->setMethod("toStringFixed", ScriptUtil::toStringFixed);
	scriptObject.getDynamicObject()->setMethod("colorToHex", ScriptUtil::colorToHex);
	scriptObject.getDynamicObject()->setMethod("getObjectProperties", ScriptUtil::getObjectProperties);
	scriptObject.getDynamicObject()->setMethod("getObjectMethods", ScriptUtil::getObjectMethods);

	scriptObject.getDynamicObject()->setMethod("getIPs", ScriptUtil::getIPs);
	scriptObject.getDynamicObject()->setMethod("encodeHMAC_SHA1", ScriptUtil::encodeHMAC_SHA1);
	scriptObject.getDynamicObject()->setMethod("encodeSHA256", ScriptUtil::encodeSHA256);
	scriptObject.getDynamicObject()->setMethod("encodeSHA512", ScriptUtil::encodeSHA512);
	scriptObject.getDynamicObject()->setMethod("toBase64", ScriptUtil::toBase64);
	scriptObject.getDynamicObject()->setMethod("fromBase64", ScriptUtil::fromBase64);
	scriptObject.getDynamicObject()->setMethod("fromBase64Bytes", ScriptUtil::fromBase64Bytes);

	scriptObject.getDynamicObject()->setMethod("fileExists", ScriptUtil::fileExistsFromScript);
	scriptObject.getDynamicObject()->setMethod("readFile", ScriptUtil::readFileFromScript);
	scriptObject.getDynamicObject()->setMethod("writeFile", ScriptUtil::writeFileFromScript);
	scriptObject.getDynamicObject()->setMethod("writeBytes", ScriptUtil::writeBytesFromScript);
	scriptObject.getDynamicObject()->setMethod("directoryExists", ScriptUtil::directoryExistsFromScript);
	scriptObject.getDynamicObject()->setMethod("createDirectory", ScriptUtil::createDirectoryFromScript);
	scriptObject.getDynamicObject()->setMethod("listFiles", ScriptUtil::listFilesFromScript);
	scriptObject.getDynamicObject()->setMethod("listDirectories", ScriptUtil::listDirectoriesFromScript);
	scriptObject.getDynamicObject()->setMethod("launchFile", ScriptUtil::launchFileFromScript);
	scriptObject.getDynamicObject()->setMethod("killApp", ScriptUtil::killAppFromScript);
	scriptObject.getDynamicObject()->setMethod("getOSInfos", ScriptUtil::getOSInfosFromScript);
	scriptObject.getDynamicObject()->setMethod("getAppVersion", ScriptUtil::getAppVersionFromScript);
	scriptObject.getDynamicObject()->setMethod("getEnvironmentVariable", ScriptUtil::getEnvironmentVariableFromScript);

	scriptObject.getDynamicObject()->setMethod("gotoURL", ScriptUtil::gotoURLFromScript);


	scriptObject.getDynamicObject()->setMethod("copyToClipboard", ScriptUtil::copyToClipboardFromScript);
	scriptObject.getDynamicObject()->setMethod("getFromClipboard", ScriptUtil::getFromClipboardFromScript);

	scriptObject.getDynamicObject()->setMethod("showMessageBox", ScriptUtil::showMessageBox);
	scriptObject.getDynamicObject()->setMethod("showOkCancelBox", ScriptUtil::showOkCancelBox);
	scriptObject.getDynamicObject()->setMethod("showYesNoCancelBox", ScriptUtil::showYesNoCancelBox);

	scriptObject.getDynamicObject()->setMethod("getSelectedObject", ScriptUtil::getSelectedObjectFromScript);
	scriptObject.getDynamicObject()->setMethod("getSelectedObjectsCount", ScriptUtil::getSelectedObjectsCountFromScript);

	scriptObject.getDynamicObject()->setMethod("loadDynamicLibrary", ScriptUtil::loadDynamicLibrary);
	scriptObject.getDynamicObject()->setMethod("unloadDynamicLibrary", ScriptUtil::unloadDynamicLibrary);
	scriptObject.getDynamicObject()->setMethod("findFunctionInDynamicLibrary", ScriptUtil::findFunctionInDynamicLibrary);
}

var ScriptUtil::getTime(const var::NativeFunctionArgs&)
{
	return (float)(Time::getMillisecondCounter() / 1000.);
}

var ScriptUtil::getTimestamp(const var::NativeFunctionArgs&)
{
	return Time::currentTimeMillis() / 1000;
}

var ScriptUtil::delayThreadMS(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return false;
	if (Thread* t = Thread::getCurrentThread())
	{
		t->wait((int)a.arguments[0]);
	}
	else
	{
		Thread::sleep((int)a.arguments[0]); //TO do better to avoid stopping UI thread
		//MessageManager::getInstance()->runDispatchLoop((int)a.arguments[0]);
	}

	return true;
}

var ScriptUtil::getFloatFromBytes(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 4) return 0;
	uint8_t bytes[4];
	for (int i = 0; i < 4; ++i) bytes[i] = (uint8_t)(int)a.arguments[i];
	float result;
	memcpy(&result, &bytes, 4);
	return result;
}

var ScriptUtil::getDoubleFromBytes(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 8) return 0;
	uint8_t bytes[8];
	for (int i = 0; i < 8; ++i) bytes[i] = (uint8_t)(int)a.arguments[i];
	double result;
	memcpy(&result, &bytes, 8);
	return result;
}

// Convert float to Hex
var ScriptUtil::floatToHexSeq(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 2) return 0;
	float value = a.arguments[0];
	bool isLittleEndian = a.arguments[1];

	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	int numBytes = sizeof(float);

	std::vector<int> result;
	result.reserve(numBytes);

	if (isLittleEndian) {
		// Little-endian
		for (int i = numBytes - 1; i >= 0; i--) {
			result.push_back(bytes[i]);
		}
	}
	else {
		// Big-endian
		for (int i = 0; i < numBytes; i++) {
			result.push_back(bytes[i]);
		}
	}

	std::ostringstream oss;

	for (int i : result) {
		oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << i;
	}

	return String(oss.str());
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

// Convert double to Hex
var ScriptUtil::doubleToHexSeq(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 2) return 0;
	double value = a.arguments[0];
	bool isLittleEndian = a.arguments[1];

	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	int numBytes = sizeof(double);

	std::vector<int> result;
	result.reserve(numBytes);

	if (isLittleEndian) {
		// Little-endian
		for (int i = numBytes - 1; i >= 0; i--) {
			result.push_back(bytes[i]);
		}
	}
	else {
		// Big-endian
		for (int i = 0; i < numBytes; i++) {
			result.push_back(bytes[i]);
		}
	}

	std::ostringstream oss;

	for (int i : result) {
		oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << i;
	}

	return String(oss.str());
}


// Convert Str Hex to Int
var ScriptUtil::hexStringToInt(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return 0;
	String hexString = a.arguments[0];

	std::stringstream ss;
	ss << std::hex << hexString;
	int result;
	ss >> result;
	return result;
}

var ScriptUtil::toStringFixed(const var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return 0;
	if (!a.arguments[0].isDouble()) return a.arguments[0].toString();
	int numDecimals = (a.numArguments > 1) ? (int)a.arguments[1] : 3;
	return String((double)a.arguments[0], numDecimals, false);
}

juce::var ScriptUtil::colorToHex(const juce::var::NativeFunctionArgs& a)
{
	if (a.arguments[0].isArray() && a.arguments[0].size() >= 3)
	{
		return Colour((float)a.arguments[0][0] * 255, (float)a.arguments[0][1] * 255, (float)a.arguments[0][2] * 255, a.arguments[0].size() > 3 ? (float)a.arguments[0][3] * 255 : 1).toDisplayString(a.arguments[0].size() > 3);
	}
	else if (a.numArguments >= 3)
	{
		return Colour((float)a.arguments[0] * 255, (float)a.arguments[1] * 255, (int)a.arguments[2] * 255, a.numArguments > 3 ? (float)a.arguments[3] * 255 : 1).toDisplayString(a.numArguments > 3);
	}

	LOGWARNING("Color conversion failed, needs either an array with 3 or 4 values (0-1) or directly 3 or 4 values as arguments");
	return "";
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

	//uint8_t* data = (uint8_t*)b.getData();
	//String dbgHex = "";
	//for (int i = 0; i < b.getSize(); ++i)
	//{
	//	dbgHex += String::toHexString(data[i]) + " ";
	//}
	return Base64::toBase64(b.getData(), b.getSize());
}

var ScriptUtil::encodeSHA256(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return "";
	String s = a.arguments[0].toString();
	return juce::SHA256(s.toStdString().c_str(), s.length()).toHexString();
}

var ScriptUtil::encodeSHA512(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return "";
	String result = OrganicCrypto::detail::basic_sha512<char>::calculate(a.arguments[0].toString().toStdString());
	return result;
}
var ScriptUtil::toBase64(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return 0;
	if (a.arguments[0].isString())
	{
		std::string result = base64_encode((const unsigned char*)a.arguments[0].toString().toStdString().c_str(), a.arguments[0].toString().length());
		return String(result);
	}
	else if (a.arguments[0].isArray())
	{
		Array<uint8> bytes;
		for (int i = 0; i < a.arguments[0].size(); i++) bytes.add((uint8)(int)a.arguments[0][i]);
		String result = Base64::toBase64(bytes.getRawDataPointer(), bytes.size());
		return result;
	}
	return "[type not handled]";
}


var ScriptUtil::fromBase64(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return 0;

	var result;
	String s = base64_decode((const std::string)a.arguments[0].toString().toStdString());
	return s;
}

var ScriptUtil::fromBase64Bytes(const var::NativeFunctionArgs& a)
{
	if (a.numArguments < 1) return 0;
	return base64_decode_bytes(a.arguments[0].toString());
}

var ScriptUtil::fileExistsFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return false;
	return File(args.arguments[0]).existsAsFile();
}

var ScriptUtil::readFileFromScript(const var::NativeFunctionArgs& args)
{
	File f = getFileFromArgs(args);
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
	File f = getFileFromArgs(args, 2);
	if (f == File()) return false;

	FileOutputStream fs(f);
	if (args.arguments[1].isObject())
	{
		JSON::writeToStream(fs, args.arguments[1]);
		return true;
	}

	return fs.writeText(args.arguments[1].toString(), false, false, "\n");
}

var ScriptUtil::writeBytesFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments < 2) return false;
	File f = getFileFromArgs(args, 2);
	if (f == File()) return false;

	FileOutputStream fs(f);
	Array<uint8_t> bytes;
	if (args.arguments[1].isArray())
	{
		for (int i = 0; i < args.arguments[1].size(); i++)
		{
			bytes.add((uint8_t)(int)args.arguments[1][i]);
		}
	}
	else
	{
		LOGWARNING("Write bytes needs an array of bytes");
		return false;
	}

	return fs.write(bytes.getRawDataPointer(), bytes.size());
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

var ScriptUtil::listFilesFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return var();

	File f = getFileFromArgs(args);

	if (!f.isDirectory()) return var();

	bool recursive = args.numArguments > 1 ? (bool)(int)args.arguments[1] : false;

	Array<File> files = f.findChildFiles(File::TypesOfFileToFind::findFiles, recursive);

	var result;
	for (auto& ff : files) result.append(ff.getFullPathName());
	return result;
}

var ScriptUtil::listDirectoriesFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return var();

	File f = getFileFromArgs(args);

	if (!f.isDirectory()) return var();

	bool recursive = args.numArguments > 1 ? (bool)(int)args.arguments[1] : false;

	Array<File> files = f.findChildFiles(File::TypesOfFileToFind::findDirectories, recursive);

	var result;
	for (auto& ff : files) result.append(ff.getFullPathName());
	return result;
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

var ScriptUtil::gotoURLFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return var();
	bool result = URL(args.arguments[0].toString()).launchInDefaultBrowser();
	return result;
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

	AlertWindow::showMessageBoxAsync(iconType, title, message, buttonText, nullptr);
	return var();
}

var ScriptUtil::showOkCancelBox(const var::NativeFunctionArgs& args)
{
	if (args.numArguments < 3) return false;

	AlertWindow::AlertIconType iconType = AlertWindow::NoIcon;
	if (args.numArguments >= 4)
	{
		String s = args.arguments[3].toString();
		if (s == "warning") iconType = AlertWindow::WarningIcon;
		if (s == "info") iconType = AlertWindow::AlertIconType::InfoIcon;
		if (s == "question") iconType = AlertWindow::AlertIconType::QuestionIcon;
	}

	String id = args.arguments[0].toString();

	String title = args.arguments[1].toString();
	String message = args.arguments[2].toString();
	String button1Text = args.numArguments >= 5 ? args.arguments[4].toString() : "";
	String button2Text = args.numArguments >= 6 ? args.arguments[5].toString() : "";

	DynamicObject* d = args.thisObject.getDynamicObject();
	if (d == nullptr) return 0;
	Script* script = dynamic_cast<Script*>((Script*)(int64)d->getProperty("_script"));
	if (script == nullptr) return 0;

	AlertWindow::showOkCancelBox(iconType, title, message, button1Text, button2Text, nullptr, ModalCallbackFunction::create([script, id](int result)
		{
			script->callFunction("messageBoxCallback", Array<var>{ id, result });
		})
	);

	return 0;
}

var ScriptUtil::showYesNoCancelBox(const var::NativeFunctionArgs& args)
{
	if (args.numArguments < 3) return false;

	AlertWindow::AlertIconType iconType = AlertWindow::NoIcon;
	if (args.numArguments >= 4)
	{
		String s = args.arguments[3].toString();
		if (s == "warning") iconType = AlertWindow::WarningIcon;
		if (s == "info") iconType = AlertWindow::AlertIconType::InfoIcon;
		if (s == "question") iconType = AlertWindow::AlertIconType::QuestionIcon;
	}

	String id = args.arguments[0].toString();
	String title = args.arguments[1].toString();
	String message = args.arguments[2].toString();
	String button1Text = args.numArguments >= 5 ? args.arguments[4].toString() : "";
	String button2Text = args.numArguments >= 6 ? args.arguments[5].toString() : "";
	String button3Text = args.numArguments >= 7 ? args.arguments[6].toString() : "";

	DynamicObject* d = args.thisObject.getDynamicObject();
	if (d == nullptr) return 0;
	Script* script = dynamic_cast<Script*>((Script*)(int64)d->getProperty("_script"));
	if (script == nullptr) return 0;

	int result = AlertWindow::showYesNoCancelBox(iconType, title, message, button1Text, button2Text, button3Text, nullptr, ModalCallbackFunction::create([script, id](int result)
		{
			script->callFunction("messageBoxCallback", Array<var>{ id, result });
		})
	);

	return result;
}

String ScriptUtil::getLogStringForVar(const var& v)
{
	String result;
	if (v.isObject())
	{
		if (v.hasProperty(scriptPtrIdentifier))
		{
			String st = v.getProperty("_type", "").toString();
			if (st.isNotEmpty())
			{
				int64 ptr = (int64)v.getProperty(scriptPtrIdentifier, 0);

				if (st == "Container")
				{
					if (ControllableContainer* cc = dynamic_cast<ControllableContainer*>(reinterpret_cast<ControllableContainer*>(ptr)))
						result += cc->getScriptTargetString();
					else
						result += "[Inaccessible Container > " + v.getProperty("name", "").toString() + "]";
				}
				else if (st == "Controllable")
				{
					if (Controllable* c = reinterpret_cast<Controllable*>(ptr))
						result += c->getScriptTargetString();
					else
						result += "[Inaccessible Controllable > " + v.getProperty("name", "").toString() + "]";
				}
			}
		}
		else if (v.isArray())
		{
			result += "[";
			for (int i = 0; i < v.size(); i++)
			{
				if (i > 0) result += ", ";
				result += getLogStringForVar(v[i]);
			}
			result += "]";
		}
		else
		{
			result += "{";
			NamedValueSet props = v.getDynamicObject()->getProperties();
			for (int i = 0; i < props.size(); i++)
			{
				if (i > 0) result += ", ";
				result += props.getName(i) + ":" + getLogStringForVar(props.getValueAt(i));
			}

			result += "}";

		}
	}
	else
	{
		result = v.toString();
	}

	return result;
}

File ScriptUtil::getFileFromArgs(const var::NativeFunctionArgs& args, int deleteIfExistsFromArg)
{
	String path = args.arguments[0].toString();

	if (!File::isAbsolutePath(path))
	{
		File folder = Engine::mainEngine->getFile().getParentDirectory();
		if (!Engine::mainEngine->getFile().existsAsFile())
		{
			String scriptPath = args.thisObject.getProperty("scriptPath", "").toString();
			folder = File(scriptPath).getParentDirectory();
		}

		path = folder.getChildFile(path).getFullPathName();
	}

	File f(path);

	if (deleteIfExistsFromArg > 0)
	{
		bool overwriteIfExists = args.numArguments > deleteIfExistsFromArg ? ((int)args.arguments[deleteIfExistsFromArg] > 0) : false;
		if (f.existsAsFile())
		{
			if (overwriteIfExists) f.deleteFile();
			else
			{
				LOG("File already exists : " << f.getFileName() << ", you need to enable overwrite to replace its content.");
				return File();
			}
		}
	}

	return f;
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
	int len = (int)data.size();
	int pad = len > 0 && (len % 4 || p[len - 1] == '=');
	const size_t L = ((len + 3) / 4 - pad) * 4;
	std::string str(L / 4 * 3 + pad, '\0');

	for (int i = 0, j = 0; i < L; i += 4)
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

var ScriptUtil::base64_decode_bytes(const String& data)
{
	const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
		7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
		0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

	CharPointer_UTF8 p = data.getCharPointer();
	//unsigned char* p = (unsigned char*)data.toStdString().c_str();
	int len = data.length();
	int pad = len > 0 && (len % 4 || p[len - 1] == '=');
	const int L = ((len + 3) / 4 - pad) * 4;
	var result;
	//result.resize(L);
	//std::string str(L / 4 * 3 + pad, '\0');

	for (int i = 0, j = 0; i < L; i += 4)
	{
		int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
		//result[j++] = n >> 16;
		//result[j++] = n >> 8 & 0xFF;
		//result[j++] = n & 0xFF;
		result.append(n >> 16);
		result.append(n >> 8 & 0xFF);
		result.append(n & 0xFF);
		j += 3;
	}
	if (pad)
	{
		int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
		result.append(n >> 16);

		if (len > L + 2 && p[L + 2] != '=')
		{
			n |= B64index[p[L + 2]] << 6;
			result.append(n >> 8 & 0xFF);
		}
	}
	return result;
}

var ScriptUtil::getSelectedObjectFromScript(const var::NativeFunctionArgs& args)
{
	BaseItem* item = nullptr;

	if (args.numArguments > 0)
	{
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		int num = args.arguments[0];
		if (num >= 0 && num < items.size())
		{
			item = items[num];
		}
	}
	else
	{
		item = InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>();
	}

	if (item) return item->getScriptObject();

	return var();
}

var ScriptUtil::getSelectedObjectsCountFromScript(const var::NativeFunctionArgs& args)
{
	return InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>().size();
}

namespace
{
	/// <summary>
	/// A dynamic library object that can be stored in Javascript.
	/// </summary>
	struct ReferenceCountedDynamicLibrary : juce::DynamicLibrary, ReferenceCountedObject
	{
		using Ptr = ReferenceCountedObjectPtr<ReferenceCountedDynamicLibrary>;
		String path;
	};
}

var ScriptUtil::loadDynamicLibrary(const var::NativeFunctionArgs& args)
{
	if (args.numArguments != 1 || !args.arguments[0].isString())
	{
		LOGERROR("Load dynamic library failed: it requires a string argument.");
		return {};
	}

	const String& path = args.arguments[0].toString();
	ReferenceCountedDynamicLibrary::Ptr dynamicLibrary = new ReferenceCountedDynamicLibrary();
	if (!dynamicLibrary->open(path))
	{
		LOGERROR("Load dynamic library failed: could not open " << path);
		return {};
	}

	dynamicLibrary->path = path;
	LOG("Dynamic library loaded: " << path);
	return dynamicLibrary.get();
}

var ScriptUtil::unloadDynamicLibrary(const var::NativeFunctionArgs& args)
{
	if (args.numArguments != 1)
	{
		LOGERROR("Unload dynamic library failed: it requires a library object argument.");
		return false;
	}

	ReferenceCountedDynamicLibrary::Ptr dynamicLibrary = dynamic_cast<ReferenceCountedDynamicLibrary*>(args.arguments[0].getObject());
	if (!dynamicLibrary)
	{
		LOGERROR("Unload dynamic library failed: it requires a library object argument.");
		return false;
	}

	dynamicLibrary->close();
	LOG("Dynamic library unloaded: " << dynamicLibrary->path);
	return {};
}

var ScriptUtil::findFunctionInDynamicLibrary(const var::NativeFunctionArgs& args)
{
	if (args.numArguments != 2)
	{
		LOGERROR("Find function in dynamic library failed: it requires a library object and a function name arguments.");
		return {};
	}

	ReferenceCountedDynamicLibrary::Ptr dynamicLibrary = dynamic_cast<ReferenceCountedDynamicLibrary*>(args.arguments[0].getObject());
	if (!dynamicLibrary || !args.arguments[1].isString())
	{
		LOGERROR("Find function in dynamic library failed: it requires a library object and a function name arguments.");
		return {};
	}

	const String functionName = args.arguments[1].toString();
	void* function = dynamicLibrary->getFunction(functionName);
	if (!function)
	{
		LOGERROR("Find function in dynamic library failed: " << functionName << " not found.\nMake sure that your function is exported and enclosed in a extern\"C\" scope.");
		return {};
	}

	// The dynamic library has its own copy of global/static variables.
	// e.g. the current custom logger registered in Chataigne won't be accessible from the library code.
	// To mitigate the issue, we pass the most important globals to the library so that it could copy them to its globals.
	//
	// StringPool::getGlobalPool() is absolutely required to read/write properties of JS objects. Indeed, property
	// lookups use identifiers that won't compare string content but pointers. Strings are stored in the global pool
	// so that a given identifier always uses the same string pointer.
	// The library receives the pointer of Chataigne's global pool to (partially) copy it to its own. Unfortunately, there
	// is no way to copy all missing entries from a pool to another one. It's the library developer responsability to copy the
	// property names that his library uses.
	using PluginFunctionType = var(*)(const var::NativeFunctionArgs&, Logger*, StringPool&);
	const auto wrapper = [function](const var::NativeFunctionArgs& args) -> var
		{
			return ((PluginFunctionType)function)(args, Logger::getCurrentLogger(), StringPool::getGlobalPool());
		};
	return var::NativeFunction{ wrapper };
}
