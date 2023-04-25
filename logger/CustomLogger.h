
#pragma once

#define MAX_LOGS 1000

class CustomLogger : public Logger
{
public:

	juce_DeclareSingleton(CustomLogger, true);

	CustomLogger();

	OwnedArray<LogElement, CriticalSection> logElements;

	void logMessage(const String& message) override;

	QueuedNotifier<String> notifier;
	typedef QueuedNotifier<String>::Listener Listener;

	const String & getWelcomeMessage();
	void addLogListener(Listener* l) { notifier.addListener(l); }
	void removeLogListener(Listener* l) { notifier.removeListener(l); }

	void setFileLogging(bool enabled);

	class FileWriter : public Listener
	{
	public:
		FileWriter();

		void newMessage(const String& s) override { if (fileLog && !s.isEmpty()) { fileLog->logMessage(s); } }
		String getFilePath() { return fileLog->getLogFile().getFullPathName(); }
		std::unique_ptr<FileLogger> fileLog;
	};

	std::unique_ptr<FileWriter> fileWriter;

private:
	const String welcomeMessage;
};
