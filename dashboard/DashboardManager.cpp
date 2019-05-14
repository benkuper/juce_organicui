/*
  ==============================================================================

    DashboardManager.cpp
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

juce_ImplementSingleton(DashboardManager)

DashboardManager::DashboardManager() :
	BaseManager("Dashboard")
{
}

DashboardManager::~DashboardManager()
{
	DashboardItemFactory::deleteInstance();
}
