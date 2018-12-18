#include "CustomLogger.h"
/*
  ==============================================================================

    Logger.cpp
    Created: 6 May 2016 1:37:41pm
    Author:  Martin Hermant

  ==============================================================================
*/

juce_ImplementSingleton(CustomLogger);

static OrganicApplication& getApp();

 CustomLogger::CustomLogger() :
	  notifier(100)
{
	addLogListener(&fileWriter);
}

void CustomLogger::logMessage(const String & message)
{
	notifier.addMessage(new String(message));
}

CustomLogger::FileWriter::FileWriter() { 
	fileLog = FileLogger::createDefaultAppLogger(getApp().getApplicationName(), "log", ""); 
}
