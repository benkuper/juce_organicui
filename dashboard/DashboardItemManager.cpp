/*
  ==============================================================================

	DashboardItemManager.cpp
	Created: 19 Apr 2017 11:06:56pm
	Author:  Ben

  ==============================================================================
*/

DashboardItemManager::DashboardItemManager() :
	BaseManager("Dashboard")
{
	managerFactory = DashboardItemFactory::getInstance();
}

DashboardItemManager::~DashboardItemManager()
{
}