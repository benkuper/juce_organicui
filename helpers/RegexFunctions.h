/** A Helper class that encapsulates the regex operations */

#pragma once

#include <regex>

class RegexFunctions
{
public:
    
    static juce::Array<juce::StringArray> findSubstringsThatMatchWildcard(const juce::String &regexWildCard, const juce::String &stringToTest)
    {
        juce::Array<juce::StringArray> matches;
        juce::String remainingText = stringToTest;
        juce::StringArray m = getFirstMatch(regexWildCard, remainingText);

        while (m.size() != 0 && m[0].length() != 0)
        {
            remainingText = remainingText.fromFirstOccurrenceOf(m[0], false, false);
            matches.add(m);
            m = getFirstMatch(regexWildCard, remainingText);
        }

        return matches;
    }

    /** Searches a string and returns a StringArray with all matches. 
    *    You can specify and index of a capture group (if not, the entire match will be used). */
    static juce::StringArray search(const juce::String& wildcard, const juce::String &stringToTest, int indexInMatch = 0)
    {
        try
        {
            juce::StringArray searchResults;

            std::regex includeRegex(wildcard.toStdString());
            std::string xAsStd = stringToTest.toStdString();
            std::sregex_iterator it(xAsStd.begin(), xAsStd.end(), includeRegex);
            std::sregex_iterator it_end;

            while (it != it_end)
            {
                std::smatch result = *it;

                juce::StringArray matches;
                for (auto x : result)
                {
                    matches.add(juce::String(x));
                }

                if (indexInMatch < matches.size()) searchResults.add(matches[indexInMatch]);

                ++it;
            }

            return searchResults;
        }
        catch (std::regex_error e)
        {
            DBG("Regex error : " << e.what());
            return juce::StringArray();
        }
    }

    static juce::Array<juce::StringArray> getAllMatches(const juce::String& wildcard, const juce::String& stringToTest)
    {
        try
        {
            juce::Array<juce::StringArray> searchResults;

            std::regex includeRegex(wildcard.toStdString());
            std::string xAsStd = stringToTest.toStdString();
            std::sregex_iterator it(xAsStd.begin(), xAsStd.end(), includeRegex);
            std::sregex_iterator it_end;

            while (it != it_end)
            {
                std::smatch result = *it;

                juce::StringArray matches;
                for (auto x : result) matches.add(juce::String(x));

                searchResults.add(matches);

                ++it;
            }

            return searchResults;
        }
        catch (std::regex_error e)
        {
            DBG("Regex error : " << e.what());
            return juce::Array<juce::StringArray>();
        }
    }

    /** Returns the first match of the given wildcard in the test string. The first entry will be the whole match, followed by capture groups. */
    static juce::StringArray getFirstMatch(const juce::String &wildcard, const juce::String &stringToTest)
    {
        try
        {
            std::regex reg(wildcard.toStdString());
            std::string s(stringToTest.toStdString());
            std::smatch match;
            

            if (std::regex_search(s, match, reg))
            {
                juce::StringArray sa;

                for (auto x:match)
                {
                    sa.add(juce::String(x));
                }
                
                return sa;
            }
            
            return juce::StringArray();
        }
        catch (std::regex_error e)
        {
            jassertfalse;

            DBG("Regex error : " << e.what());
            return juce::StringArray();
        }
    }
    
    /** Checks if the given string matches the regex wildcard. */
    static bool matchesWildcard(const juce::String &wildcard, const juce::String &stringToTest)
    {        
        try
        {
            std::regex reg(wildcard.toStdString());
            
            return std::regex_search(stringToTest.toStdString(), reg);
        }
        catch (std::regex_error e)
        {
            DBG("Regex error : " << e.what());
            
            return false;
        }
    }

};