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
	static var getTimestamp(const var::NativeFunctionArgs &a);
	static var getFloatFromBytes(const var::NativeFunctionArgs &a);
	static var getInt32FromBytes(const var::NativeFunctionArgs &a);
	static var getInt64FromBytes(const var::NativeFunctionArgs &a);

	static var getIPs(const var::NativeFunctionArgs& a);

	static var encodeHMAC_SHA1(const var::NativeFunctionArgs& a);

	static var toBase64(const var::NativeFunctionArgs& a);

	static var readFileFromScript(const var::NativeFunctionArgs& args);
	static var writeFileFromScript(const var::NativeFunctionArgs& args);
	static var createDirectoryFromScript(const var::NativeFunctionArgs& args);
};
