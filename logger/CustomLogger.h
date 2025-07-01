
#pragma once

#define MAX_LOGS 1000

class CustomLogger : public juce::Logger
{
public:

	juce_DeclareSingleton(CustomLogger, true);

	CustomLogger();

	juce::OwnedArray<LogElement, juce::CriticalSection> logElements;

	void logMessage(const juce::String& message) override;

	struct LogEvent
	{
		juce::WeakReference<LogElement> log;
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

		void newMessage(const LogEvent& e) override { if (fileLog && !e.log->content.isEmpty()) { fileLog->logMessage(e.log->getFullLog()); } }
		juce::String getFilePath() { return fileLog->getLogFile().getFullPathName(); }
		std::unique_ptr<juce::FileLogger> fileLog;
	};

	std::unique_ptr<FileWriter> fileWriter;

private:
	const juce::String welcomeMessage;
};
