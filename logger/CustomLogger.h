
#pragma once

#define MAX_LOGS 2000

class CustomLogger : public juce::Logger
{
public:

	juce_DeclareSingleton(CustomLogger, true);

	CustomLogger();

	juce::OwnedArray<LogElement, juce::CriticalSection> logElements;

	void logMessage(const juce::String& message) override;

	struct LogEvent
	{
		LogEvent(LogElement* l) : time(l->time), content(l->getContent()), source(l->source), severity(l->severity), severityName(l->getSeverityName()) {
			contentLines = juce::StringArray::fromLines(content);
			numLines = contentLines.size();
		}
		~LogEvent() {}
		LogEvent(juce::Time time, juce::String content, juce::String source, LogElement::Severity severity, juce::String severityName) :
			time(time), content(content), source(source), severity(severity), severityName(severityName) {
			contentLines = juce::StringArray::fromLines(content);
			numLines = contentLines.size();
		}

		juce::Time time;
		juce::String content;
		juce::StringArray contentLines;
		juce::String source;
		LogElement::Severity severity;
		juce::String severityName;
		int numLines;
		juce::String getFullLog() const { return time.toString(false, true, true, true) + " [" + severityName + "] " + source + " : " + content; }
	};

	QueuedNotifier<LogEvent> notifier;
	typedef QueuedNotifier<LogEvent>::Listener LoggerListener;

	const juce::String& getWelcomeMessage();
	void addLogListener(LoggerListener* l) { notifier.addListener(l); }
	void removeLogListener(LoggerListener* l) { notifier.removeListener(l); }

	void setFileLogging(bool enabled);

	class FileWriter : public LoggerListener
	{
	public:
		FileWriter();

		void newMessage(const LogEvent& e) override { if (fileLog && !e.content.isEmpty()) { fileLog->logMessage(e.getFullLog()); } }
		juce::String getFilePath() { return fileLog->getLogFile().getFullPathName(); }
		std::unique_ptr<juce::FileLogger> fileLog;
	};

	std::unique_ptr<FileWriter> fileWriter;

private:
	const juce::String welcomeMessage;
};
