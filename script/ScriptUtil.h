/*
  ==============================================================================

	ScriptUtil.h
	Created: 21 Feb 2017 8:44:28am
	Author:  Ben

  ==============================================================================
*/

#pragma once

class ScriptUtil :
	public ScriptTarget
{
public:
	juce_DeclareSingleton(ScriptUtil, true)

	ScriptUtil();
	~ScriptUtil() {}

	static var getTime(const var::NativeFunctionArgs& a);
	static var getTimestamp(const var::NativeFunctionArgs& a);
	static var delayThreadMS(const var::NativeFunctionArgs& a);


	static var getFloatFromBytes(const var::NativeFunctionArgs& a);
	static var floatToHexSeq(const var::NativeFunctionArgs& a);
	static var getInt32FromBytes(const var::NativeFunctionArgs& a);
	static var getInt64FromBytes(const var::NativeFunctionArgs& a);
	static var doubleToHexSeq(const var::NativeFunctionArgs& a);
	static var hexStringToInt(const var::NativeFunctionArgs& a);
	static var toStringFixed(const var::NativeFunctionArgs& a);

	static var getObjectProperties(const var::NativeFunctionArgs& a);
	static var getObjectMethods(const var::NativeFunctionArgs& a);

	static var getIPs(const var::NativeFunctionArgs& a);

	static var encodeHMAC_SHA1(const var::NativeFunctionArgs& a);
	static var encodeSHA256(const var::NativeFunctionArgs& a);
	static var encodeSHA512(const var::NativeFunctionArgs& a);

	static var toBase64(const var::NativeFunctionArgs& a);
	static var fromBase64(const var::NativeFunctionArgs& a);
	static var fromBase64Bytes(const var::NativeFunctionArgs& a);

	static var fileExistsFromScript(const var::NativeFunctionArgs& args);
	static var readFileFromScript(const var::NativeFunctionArgs& args);
	static var writeFileFromScript(const var::NativeFunctionArgs& args);
	static var writeBytesFromScript(const var::NativeFunctionArgs& args);
	static var directoryExistsFromScript(const var::NativeFunctionArgs& args);
	static var createDirectoryFromScript(const var::NativeFunctionArgs& args);
	static var listFilesFromScript(const var::NativeFunctionArgs& args);
	static var listDirectoriesFromScript(const var::NativeFunctionArgs& args);
	static var launchFileFromScript(const var::NativeFunctionArgs& args);
	static var killAppFromScript(const var::NativeFunctionArgs& args);
	static var getOSInfosFromScript(const var::NativeFunctionArgs& args);
	static var getAppVersionFromScript(const var::NativeFunctionArgs& args);
	static var getEnvironmentVariableFromScript(const var::NativeFunctionArgs& args);

	static var gotoURLFromScript(const var::NativeFunctionArgs& args);

	static var copyToClipboardFromScript(const var::NativeFunctionArgs& args);
	static var getFromClipboardFromScript(const var::NativeFunctionArgs& args);


	static var showMessageBox(const var::NativeFunctionArgs& args);
	static var showOkCancelBox(const var::NativeFunctionArgs& args);
	static var showYesNoCancelBox(const var::NativeFunctionArgs& args);


	//Helpers

	static String getLogStringForVar(const var & v);

	static File getFileFromArgs(const var::NativeFunctionArgs& args, int deleteIfExistFromArg = -1);

	static std::string base64_encode(unsigned char const* src, unsigned int len);
	static std::string base64_decode(std::string const& data);
	static var base64_decode_bytes(const String & data);
	
	static var getSelectedObjectFromScript(const var::NativeFunctionArgs& args);
	static var getSelectedObjectsCountFromScript(const var::NativeFunctionArgs& args);
};
