/*
  ==============================================================================

    ProjectSettings.cpp
    Created: 22 Nov 2017 11:30:40am
    Author:  Ben

  ==============================================================================
*/

#include "ProjectSettings.h"

juce_ImplementSingleton(ProjectSettings)

ProjectSettings::ProjectSettings() :
	ControllableContainer("Project Settings"),
	dashboardCC("Dashboard Settings")
{
	saveAndLoadRecursiveData = true;
	fullScreenOnStartup = addBoolParameter("Full Screen on load", "If checked, the app will go full screen when loading the file, otherwise it will go to window mode", false);
	
	saveLayoutReference = addBoolParameter("Save Layout in file", "If checked, this will save the layout in the .noisette file", true);

	showDashboardOnStartup = dashboardCC.addTargetParameter("Show Dashboard on startup", "If enabled, the selected dashboard will be the one displayed on startup", DashboardManager::getInstance(), false);
	showDashboardOnStartup->targetType = TargetParameter::CONTAINER;
	showDashboardOnStartup->canBeDisabledByUser = true;
	showDashboardOnStartup->maxDefaultSearchLevel = 0;

#if ORGANICUI_USE_WEBSERVER
	enableServer = dashboardCC.addBoolParameter("Enable Dashboard Server", "Activates / Deactivates exposing Dashboard as a webserver", false);
	serverPort = dashboardCC.addIntParameter("Server Port", "The port that the server binds to", 9999, 0, 65535, false);
#endif

	addChildControllableContainer(&dashboardCC);

}

ProjectSettings::~ProjectSettings()
{
}

void ProjectSettings::reset()
{
	fullScreenOnStartup->resetValue();
	showDashboardOnStartup->resetValue();
}

void ProjectSettings::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
#if ORGANICUI_USE_WEBSERVER
	if (c == enableServer || c == serverPort)
	{
		if(c == enableServer) serverPort->setEnabled(enableServer->boolValue());
		DashboardManager::getInstance()->setupServer();
	}
#endif
}
