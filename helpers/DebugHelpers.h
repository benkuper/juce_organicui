/*
  ==============================================================================

    DebugHelpers.h
    Created: 6 May 2016 2:10:12pm
    Author:  Martin Hermant

  ==============================================================================
*/

#pragma once

//  header (build and link-time cheap) to include everywhere we need specific debug macros



// slower but safe log (avoid flooding
#define SLOG(textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf;\
static uint32 lastTime =  juce::Time::getMillisecondCounter(); \
static bool runningUnderDebugger = juce_isRunningUnderDebugger();\
uint32 now = juce::Time::getMillisecondCounter();\
if( (now - lastTime>300 )|| runningUnderDebugger){ \
juce::String fullPath = juce::String(__FILE__);\
tempDbgBuf << fullPath.substring (fullPath.lastIndexOfChar (juce::File::getSeparatorChar()) + 1 ,fullPath.lastIndexOfChar('.') ) << "::" <<  textToWrite;\
juce::Logger::writeToLog(tempDbgBuf);\
lastTime = now;})

// log informing file from where it was outputed
#define LOG(textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf;\
juce::String fullPath = juce::String(__FILE__);\
tempDbgBuf << fullPath.substring (fullPath.lastIndexOfChar (juce::File::getSeparatorChar()) + 1 ,fullPath.lastIndexOfChar('.') ) << "::" <<  textToWrite;\
juce::Logger::writeToLog(tempDbgBuf);)

#define LOGWARNING(textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf;\
juce::String fullPath = juce::String(__FILE__);\
tempDbgBuf << fullPath.substring (fullPath.lastIndexOfChar (juce::File::getSeparatorChar()) + 1 ,fullPath.lastIndexOfChar('.') ) << "::!!" <<  textToWrite;\
juce::Logger::writeToLog(tempDbgBuf);)

#define LOGERROR(textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf;\
juce::String fullPath = juce::String(__FILE__);\
tempDbgBuf<< fullPath.substring (fullPath.lastIndexOfChar (juce::File::getSeparatorChar()) + 1 ,fullPath.lastIndexOfChar('.') ) << "::!!!"  <<  textToWrite;\
juce::Logger::writeToLog(tempDbgBuf);)


// named version where source name is user defined
#define NLOG(__name,textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf;\
tempDbgBuf << __name << juce::StringRef("::") << textToWrite;\
juce::Logger::writeToLog(tempDbgBuf);)

#define NLOGWARNING(__name,textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf;\
tempDbgBuf << __name << juce::StringRef("::!!") << textToWrite;\
juce::Logger::writeToLog(tempDbgBuf);)

#define NLOGERROR(__name,textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf;\
tempDbgBuf << __name << juce::StringRef("::!!!") << textToWrite;\
juce::Logger::writeToLog(tempDbgBuf);)




inline juce::String getLogSource(const juce::String & logString) {
	return logString.substring(0, logString.indexOf("::"));// .trim();
}


inline juce::String getLogContent(const juce::String & logString) {
  int startString = logString.indexOf("::");
  if (startString>=0)startString+=2;
  else startString=0;
    return logString.substring( startString,logString.length());//.trim();
}


class LogElement
{
public:
	LogElement(juce::String log) :
		content(getLogContent(log)),
        source(getLogSource(log))
		
	{
		_arr.reset(new juce::StringArray());
		time = juce::Time::getCurrentTime();
		//_arr->addTokens(content, StringRef("\r\n"), StringRef("\""));
		_arr->addLines(content);
		if (_arr->size())
		{
			juce::String* s = &_arr->getReference(0);
			auto cp = s->getCharPointer();
			severity = LOG_NONE;

			while (cp.getAndAdvance() == '!' && severity < LOG_ERR)
			{
				severity = (Severity)(severity + 1);
			}

			if (severity == LOG_NONE && s->startsWith("JUCE Assertion"))
			{
				severity = LOG_ERR;
			}
			else
			{
				_arr->set(0, _arr->getReference(0).substring((int)severity + 1));
			}

		}
		else
		{
			severity = LOG_NONE;
		}
	}
	juce::Time time;
	juce::String content;
	juce::String source;
	enum Severity { LOG_NONE = -1, LOG_DBG = 0, LOG_WARN = 1, LOG_ERR = 2 };
	Severity severity;
	int getNumLines() const { return  _arr->size(); }
	void trimToFit(int num) { if (_arr->size() > num)_arr->removeRange(0, _arr->size() - num); }
	const juce::String& getLine(int i) const { return _arr->getReference(i); }


private:
	std::unique_ptr<juce::StringArray> _arr;

};