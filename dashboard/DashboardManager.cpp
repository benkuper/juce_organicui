/*
  ==============================================================================

    DashboardManager.cpp
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(DashboardManager)

DashboardManager::DashboardManager() :
	BaseManager("Dashboards")
{
	skipLabelInTarget = false;
	editMode = addBoolParameter("Edit Mode", "If checked, items are editable. If not, items are normally usable", true);
	snapping = addBoolParameter("Snapping", "If checked, items are automatically aligned when dragging them closed to other ones", true);
}

DashboardManager::~DashboardManager()
{
	DashboardItemFactory::deleteInstance();
}
