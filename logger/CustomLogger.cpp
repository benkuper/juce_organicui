juce_ImplementSingleton(CustomLogger);

CustomLogger::CustomLogger() :
	notifier(2000, false),
	welcomeMessage(getApp().getApplicationName() + " v" + String(ProjectInfo::versionString) + " : (" + String(Time::getCompilationDate().formatted("%d/%m/%y (%R)")) + ")")
 {
	
#if USE_FILE_LOGGER
	addLogListener(&fileWriter);
#endif

}

const String & CustomLogger::getWelcomeMessage() {
	return welcomeMessage;
}


void CustomLogger::logMessage(const String& message)
{
	notifier.addMessage(new String(message));
	DBG(message);
}

#if USE_FILE_LOGGER
CustomLogger::FileWriter::FileWriter() 
{
	fileLog.reset(FileLogger::createDefaultAppLogger(getApp().getApplicationName(), "log", ""));
}
#endif