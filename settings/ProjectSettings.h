/*
  ==============================================================================

    ProjectSettings.h
    Created: 22 Nov 2017 11:30:40am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ProjectSettings :
	public ControllableContainer
{
public:
	juce_DeclareSingleton(ProjectSettings, true);
	ProjectSettings();
	~ProjectSettings();

	BoolParameter* fullScreenOnStartup;
	TargetParameter* showDashboardOnStartup;
	BoolParameter* saveLayoutReference;

	void reset();
};