/*
 ==============================================================================

 StringUtil.h
 Created: 8 Mar 2016 2:43:25pm
 Author:  bkupe

 ==============================================================================
 */

#pragma once

#pragma warning(push)
#pragma warning(disable:4245)

#include "JuceHeader.h"

using namespace juce;

String StringUtil::toShortName(const String& niceName, bool replaceSlashes) {
	if (niceName.isEmpty()) return "";


	String res = niceName;
	if (replaceSlashes) res = res.replaceCharacter('/', '_');

	res = res.replaceCharacter('\"', '_');

	String specials = "+-()[]{}<>^'@#*$~";// ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöùúûüýÿ";
	String replaces = "_____________#___";//AAAAAAECEEEEIIIIDNOOOOOxUUUUYsaaaaaaeceeeeiiiiOnooooouuuuyy";
	res = res.replaceCharacters(specials, replaces);

#if !JUCE_MAC
	res = res.replaceCharacter('â', 'a').replaceCharacter('é', 'e').replaceCharacter('è', 'e').replaceCharacter('ê', 'e')
		.replaceCharacter('à', 'a').replaceCharacter('ô', 'o')
		.replaceCharacter('ç', 'c').replaceCharacter('ü', 'u');
#endif

	//for (int i = 0; i < specials.length(); ++i) res = res.replaceCharacter(specials[i], replaces[i]);

	res = res.retainCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789# ");

	StringArray sa;
	sa.addTokens(res, false);
	int index = 0;

	for (auto& s : sa.strings)
	{
		if (s.isEmpty()) continue;
		if (index == 0 && s.toUpperCase() == s)
		{
			String lowerFull = s.toLowerCase();
			s.swapWith(lowerFull);
		}
		else
		{
			String initial = s.substring(0, 1);
			String upperCaseWord = s.replaceSection(0, 1, index == 0 ? initial.toLowerCase() : initial.toUpperCase());
			s.swapWith(upperCaseWord);
		}

		index++;
	}

	return sa.joinIntoString("");
}

#pragma warning(pop)

CommandLineElements StringUtil::parseCommandLine(const String& commandLine) {
	CommandLineElements res;
	StringArray args;
	args.addTokens(commandLine, true);
	args.trim();

	int parsingIdx = 0;

	while (parsingIdx < args.size())
	{
		String command = "";
		bool isParameter = args[parsingIdx].startsWith("-");
		if (isParameter) {
			command = args[parsingIdx].substring(1, args[parsingIdx].length());
			parsingIdx++;
			res.add(CommandLineElement(command));
			// handles command only args
			if (parsingIdx >= args.size()) { break; }
		}

		String argument = args[parsingIdx].removeCharacters(juce::StringRef("\""));

		// handles no command args at the begining
		if (res.size() == 0) { res.add(CommandLineElement()); }

		(res.end() - 1)->args.add(argument);
		//DBG("parsing commandline, command : " << command << ", argument :" << argument << " / parsingIdx : " << parsingIdx);

		parsingIdx++;
	}
	return res;

}

#pragma warning (push)
#pragma warning(disable:4244)
String StringUtil::valueToTimeString(double timeVal, int numDecimals)
{
	String sign = timeVal < 0 ? "-" : "";
	int hours = abs(trunc(timeVal / 3600));
	int minutes = abs(trunc(fmod(timeVal, 3600) / 60));
	double seconds = abs(fmod(timeVal, 60));
	if (numDecimals > 0) return String::formatted("{}%02i:%02i:%0" + String(3 + numDecimals) + "." + String(numDecimals) + "f", sign, hours, minutes, seconds);
	else return String::formatted("{}%02i:%02i:%02i", sign, hours, minutes, (int)seconds);
}

double StringUtil::timeStringToValue(String str)
{
	StringArray sa;
	str = str.retainCharacters("0123456789.:;,");
	if (str.endsWithChar(':')) str += "0";
	sa.addTokens(str.replace(",", "."), ":", "");

	double value = 0;

	value += sa.strings[sa.strings.size() - 1].getFloatValue();

	if (sa.strings.size() >= 2)
	{
		int numMinutes = sa.strings[sa.strings.size() - 2].getIntValue() * 60;
		value += numMinutes;
		if (sa.strings.size() >= 3)
		{
			int numHours = sa.strings[sa.strings.size() - 3].getFloatValue() * 3600;
			value += numHours;
		}
	}

	return value;
}


