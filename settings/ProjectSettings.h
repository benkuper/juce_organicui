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
	ControllableContainer dashboardDefaults;

	Point2DParameter* triggerDefaultSize;
	Point2DParameter* boolDefaultSize;
	EnumParameter* boolDefaultStyle;
	Point2DParameter* floatDefautSize;
	EnumParameter* floatDefaultStyle;
	Point2DParameter* intDefautSize;
	EnumParameter* intDefaultStyle;
	Point2DParameter* stringDefaultSize;
	Point2DParameter* enumDefaultSize;
	EnumParameter* enumDefaultStyle;
	Point2DParameter* colorDefaultSize;
	Point2DParameter* targetDefaultSize;
	Point2DParameter* p2dDefaultSize;
	Point2DParameter* p3dDefaultSize;

	ControllableContainer customValuesCC;
	ControllableContainer customRangesCC;

	void reset();
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;
};