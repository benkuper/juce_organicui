/*
  ==============================================================================

	ProjectSettings.cpp
	Created: 22 Nov 2017 11:30:40am
	Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(ProjectSettings)

ProjectSettings::ProjectSettings() :
	ControllableContainer("Project Settings"),
	dashboardCC("Dashboard Settings"),
	customValuesCC("Custom Definitions"),
	customRangesCC("Custom Ranges"),
	dashboardDefaults("Defaults Values"),
	unlockOnce(nullptr),
	dashboardPassword(nullptr)
{
	saveAndLoadRecursiveData = true;
	fullScreenOnStartup = addBoolParameter("Full Screen on load", "If checked, the app will go full screen when loading the file, otherwise it will go to window mode", false);

	saveLayoutReference = addBoolParameter("Save Layout in file", "If checked, this will save the layout in the .noisette file", true);

	showDashboardOnStartup = dashboardCC.addTargetParameter("Show Dashboard on startup", "If enabled, the selected dashboard will be the one displayed on startup", DashboardManager::getInstance(), false);
	showDashboardOnStartup->targetType = TargetParameter::CONTAINER;
	showDashboardOnStartup->canBeDisabledByUser = true;
	showDashboardOnStartup->maxDefaultSearchLevel = 0;



	triggerDefaultSize = dashboardDefaults.addPoint2DParameter("Trigger Size", "Default Size for this type", false);
	boolDefaultSize = dashboardDefaults.addPoint2DParameter("Bool Size", "Default Size for this type", false);
	boolDefaultStyle = dashboardDefaults.addEnumParameter("Bool Style", "Default style for this type", false);
	floatDefautSize = dashboardDefaults.addPoint2DParameter("Float Size", "Default Size for this type", false);
	floatDefaultStyle = dashboardDefaults.addEnumParameter("Float Style", "Default style for this type", false);
	intDefautSize = dashboardDefaults.addPoint2DParameter("Int Size", "Default size for this type", false);
	intDefaultStyle = dashboardDefaults.addEnumParameter("Int Style", "Default style for this type", false);
	stringDefaultSize = dashboardDefaults.addPoint2DParameter("String Size", "Default size for this type", false);
	enumDefaultSize = dashboardDefaults.addPoint2DParameter("Enum Style", "Default style for this type", false);
	enumDefaultStyle = dashboardDefaults.addEnumParameter("Enum Size", "Default size for this type", false);
	colorDefaultSize = dashboardDefaults.addPoint2DParameter("Color Size", "Default size for this type", false);
	targetDefaultSize = dashboardDefaults.addPoint2DParameter("Target Size", "Default size for this type", false);
	p2dDefaultSize = dashboardDefaults.addPoint2DParameter("Point2D Size", "Default size for this type", false);
	p3dDefaultSize = dashboardDefaults.addPoint2DParameter("Point3D Size", "Default size for this type", false);

	triggerDefaultSize->canBeDisabledByUser = true;
	boolDefaultSize->canBeDisabledByUser = true;
	boolDefaultStyle->canBeDisabledByUser = true;
	floatDefautSize->canBeDisabledByUser = true;
	floatDefaultStyle->canBeDisabledByUser = true;
	intDefautSize->canBeDisabledByUser = true;
	intDefaultStyle->canBeDisabledByUser = true;
	stringDefaultSize->canBeDisabledByUser = true;
	enumDefaultSize->canBeDisabledByUser = true;
	enumDefaultStyle->canBeDisabledByUser = true;
	colorDefaultSize->canBeDisabledByUser = true;
	targetDefaultSize->canBeDisabledByUser = true;
	p2dDefaultSize->canBeDisabledByUser = true;
	p3dDefaultSize->canBeDisabledByUser = true;

	triggerDefaultSize->setPoint(100, 20);
	boolDefaultSize->setPoint(100, 20);
	boolDefaultStyle->addOption("Default", -1)->addOption("Toggle Checkbox", 22)->addOption("Toggle Button", 23)->addOption("Momentary Checkbox", 24)->addOption("Momentary Button", 25)->addOption("Color Circle", 10)->addOption("Color Square", 11);
	floatDefautSize->setPoint(100, 20);
	floatDefaultStyle->addOption("Default", -1)->addOption("Horizontal Slider", 0)->addOption("Vertical Slider", 1)->addOption("Rotary Slider", 5)->addOption("Text", 2)->addOption("Time", 3)->addOption("Color Circle", 10)->addOption("Color Square", 11);
	intDefautSize->setPoint(100, 20);
	intDefaultStyle->addOption("Default", -1)->addOption("Horizontal Slider", 0)->addOption("Vertical Slider", 1)->addOption("Rotary Slider", 5)->addOption("Text", 2)->addOption("Time", 3);
	stringDefaultSize->setPoint(100, 20);
	enumDefaultSize->setPoint(100, 20);
	enumDefaultStyle->addOption("Default", -1)->addOption("Horizontal Bar", 20)->addOption("Vertical Bar", 21);
	colorDefaultSize->setPoint(100, 20);
	targetDefaultSize->setPoint(100, 20);
	p2dDefaultSize->setPoint(100, 20);
	p3dDefaultSize->setPoint(100, 20);

	dashboardDefaults.editorIsCollapsed = true;
	dashboardCC.addChildControllableContainer(&dashboardDefaults);

#if ORGANICUI_USE_WEBSERVER
	enableServer = dashboardCC.addBoolParameter("Enable Dashboard Server", "Activates / Deactivates exposing Dashboard as a webserver", false);
	serverPort = dashboardCC.addIntParameter("Server Port", "The port that the server binds to", 9999, 0, 65535, false);
	dashboardPassword = dashboardCC.addStringParameter("Password", "Password for web clients to access the dashboard, leave empty public access", "");
	unlockOnce = dashboardCC.addBoolParameter("Unlock Only Once", "If checked, this will allow to only have to unlock once per session. Refreshing the page will reset the lock.", false);
#endif



	addChildControllableContainer(&dashboardCC);

	customRangesCC.userCanAddControllables = true;
	customRangesCC.userAddControllablesFilters.add(Point2DParameter::getTypeStringStatic());

	customValuesCC.addChildControllableContainer(&customRangesCC);
	addChildControllableContainer(&customValuesCC);
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
		if (c == enableServer) serverPort->setEnabled(enableServer->boolValue());
		DashboardManager::getInstance()->setupServer();
	}
#endif
}
