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

	static juce::var getTime(const juce::var::NativeFunctionArgs& a);
	static juce::var getTimestamp(const juce::var::NativeFunctionArgs& a);
	static juce::var delayThreadMS(const juce::var::NativeFunctionArgs& a);


	static juce::var getFloatFromBytes(const juce::var::NativeFunctionArgs& a);
	static juce::var getDoubleFromBytes(const juce::var::NativeFunctionArgs& a);
	static juce::var floatToHexSeq(const juce::var::NativeFunctionArgs& a);
	static juce::var getInt32FromBytes(const juce::var::NativeFunctionArgs& a);
	static juce::var getInt64FromBytes(const juce::var::NativeFunctionArgs& a);
	static juce::var doubleToHexSeq(const juce::var::NativeFunctionArgs& a);
	static juce::var hexStringToInt(const juce::var::NativeFunctionArgs& a);
	static juce::var toStringFixed(const juce::var::NativeFunctionArgs& a);
	static juce::var colorToHex(const juce::var::NativeFunctionArgs& a);

	static juce::var getObjectProperties(const juce::var::NativeFunctionArgs& a);
	static juce::var getObjectMethods(const juce::var::NativeFunctionArgs& a);

	static juce::var getIPs(const juce::var::NativeFunctionArgs& a);

	static juce::var encodeHMAC_SHA1(const juce::var::NativeFunctionArgs& a);
	static juce::var encodeSHA256(const juce::var::NativeFunctionArgs& a);
	static juce::var encodeSHA512(const juce::var::NativeFunctionArgs& a);

	static juce::var toBase64(const juce::var::NativeFunctionArgs& a);
	static juce::var fromBase64(const juce::var::NativeFunctionArgs& a);
	static juce::var fromBase64Bytes(const juce::var::NativeFunctionArgs& a);

	static juce::var getDocumentsDirectoryFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getDesktopDirectoryFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getCurrentFileDirectoryFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getCurrentFilePathFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var fileExistsFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getNonExistentFileFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getFileName(const juce::var::NativeFunctionArgs& args);
	static juce::var getFilePath(const juce::var::NativeFunctionArgs& args);
	static juce::var getParentDirectory(const juce::var::NativeFunctionArgs& args);
	static juce::var readFileFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var writeFileFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var writeBytesFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var directoryExistsFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var createDirectoryFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var listFilesFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var listDirectoriesFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var launchFileFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var killAppFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getOSInfosFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getAppVersionFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getEnvironmentVariableFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var gotoURLFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var copyToClipboardFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getFromClipboardFromScript(const juce::var::NativeFunctionArgs& args);


	static juce::var showMessageBox(const juce::var::NativeFunctionArgs& args);
	static juce::var showOkCancelBox(const juce::var::NativeFunctionArgs& args);
	static juce::var showYesNoCancelBox(const juce::var::NativeFunctionArgs& args);

	static juce::var loadDynamicLibrary(const juce::var::NativeFunctionArgs& args);
	static juce::var unloadDynamicLibrary(const juce::var::NativeFunctionArgs& args);
	static juce::var findFunctionInDynamicLibrary(const juce::var::NativeFunctionArgs& args);


	//Helpers

	static juce::String getLogStringForVar(const juce::var & v);

	static juce::File getFileFromArgs(const juce::var::NativeFunctionArgs& args, int deleteIfExistFromArg = -1);

	static std::string base64_encode(unsigned char const* src, unsigned int len);
	static std::string base64_decode(std::string const& data);
	static juce::var base64_decode_bytes(const juce::String & data);
	
	static juce::var getSelectedObjectFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getSelectedObjectsCountFromScript(const juce::var::NativeFunctionArgs& args);
};
