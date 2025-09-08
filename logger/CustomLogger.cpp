
juce_ImplementSingleton(CustomLogger);

CustomLogger::CustomLogger() :
	notifier(5000, false),
	welcomeMessage(getApp().getApplicationName() + " v" + String(ProjectInfo::versionString) + " : (" + String(Time::getCompilationDate().formatted("%d/%m/%y (%R)")) + ")")
{

}

const String & CustomLogger::getWelcomeMessage() {
	return welcomeMessage;
}


void CustomLogger::logMessage(const String& message)
{
	LogElement* el = new LogElement(message);
	while (logElements.size() >= MAX_LOGS)
	{
		notifier.cancelPendingUpdate();
		logElements.remove(0, true);
	}

	logElements.add(el);
	notifier.addMessage(new LogEvent(el));
	DBG(message);
}

void CustomLogger::setFileLogging(bool enabled)
{
	if(enabled && !fileWriter){
		fileWriter.reset(new CustomLogger::FileWriter());
		addLogListener(fileWriter.get());
	}
	else if(fileWriter)
	{
		removeLogListener(fileWriter.get());
		fileWriter.reset();
	}
}

CustomLogger::FileWriter::FileWriter() 
{
	fileLog.reset(FileLogger::createDateStampedLogger(getApp().getApplicationName(), "log_", ".txt", ""));
}
