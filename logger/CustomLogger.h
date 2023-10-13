
#pragma once

#define MAX_LOGS 1000

class CustomLogger : public juce::Logger
{
public:

	juce_DeclareSingleton(CustomLogger, true);

	CustomLogger();

	juce::OwnedArray<LogElement, juce::CriticalSection> logElements;

	void logMessage(const juce::String& message) override;

	QueuedNotifier<juce::String> notifier;
	typedef QueuedNotifier<juce::String>::Listener Listener;

	const juce::String & getWelcomeMessage();
	void addLogListener(Listener* l) { notifier.addListener(l); }
	void removeLogListener(Listener* l) { notifier.removeListener(l); }

	void setFileLogging(bool enabled);

	class FileWriter : public Listener
	{
	public:
		FileWriter();

		void newMessage(const juce::String& s) override { if (fileLog && !s.isEmpty()) { fileLog->logMessage(s); } }
		juce::String getFilePath() { return fileLog->getLogFile().getFullPathName(); }
		std::unique_ptr<juce::FileLogger> fileLog;
	};

	std::unique_ptr<FileWriter> fileWriter;

private:
	const juce::String welcomeMessage;
};
