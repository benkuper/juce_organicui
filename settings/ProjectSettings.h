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
	BoolParameter* saveLayoutReference;

	ControllableContainer dashboardCC;
	TargetParameter* showDashboardOnStartup;
	BoolParameter* enableServer;
	IntParameter* serverPort;
	StringParameter* dashboardPassword;

	void reset();

	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;
};