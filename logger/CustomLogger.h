

#pragma once

 // do not use file logger atm
 // TODO figure out true utility of such
#define USE_FILE_LOGGER 0

class CustomLogger : public Logger
{
public:

	juce_DeclareSingleton(CustomLogger, true);

	CustomLogger();

	void logMessage(const String& message) override;


	QueuedNotifier<String> notifier;
	typedef QueuedNotifier<String>::Listener Listener;


	const String & getWelcomeMessage();
	void addLogListener(Listener* l) { notifier.addListener(l); }
	void removeLogListener(Listener* l) { notifier.removeListener(l); }

#if USE_FILE_LOGGER
	class FileWriter : public Listener
	{
	public:
		FileWriter() { fileLog = FileLogger::createDefaultAppLogger(getApp().getApplicationName(), "log", ""); }

		void newMessage(const String& s) override { if (fileLog && !s.isEmpty()) { fileLog->logMessage(s); } }
		String getFilePath() { return fileLog->getLogFile().getFullPathName(); }
		std::unique_ptr<FileLogger> fileLog;
	};

	FileWriter fileWriter;
#endif

private:
	const String welcomeMessage;
};
