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
	ControllableContainer("Project Settings")
{
	saveAndLoadRecursiveData = true;
	fullScreenOnStartup = addBoolParameter("Full Screen on load", "If checked, the app will go full screen when loading the file, otherwise it will go to window mode", false);
	showDashboardOnStartup = addTargetParameter("Show Dashboard on startup", "If enabled, the selected dashboard will be the one displayed on startup", DashboardManager::getInstance(), false);
	showDashboardOnStartup->targetType = TargetParameter::CONTAINER;
	showDashboardOnStartup->canBeDisabledByUser = true;
	showDashboardOnStartup->maxDefaultSearchLevel = 0;

}

ProjectSettings::~ProjectSettings()
{
}

void ProjectSettings::reset()
{
	fullScreenOnStartup->resetValue();
	showDashboardOnStartup->resetValue();

}
