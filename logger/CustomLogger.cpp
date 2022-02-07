juce_ImplementSingleton(CustomLogger);

CustomLogger::CustomLogger() :
	notifier(2000, false),
	welcomeMessage(getApp().getApplicationName() + " v" + String(ProjectInfo::versionString) + " : (" + String(Time::getCompilationDate().formatted("%d/%m/%y (%R)")) + ")")
{

}

const String & CustomLogger::getWelcomeMessage() {
	return welcomeMessage;
}


void CustomLogger::logMessage(const String& message)
{
	notifier.addMessage(new String(message));
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
