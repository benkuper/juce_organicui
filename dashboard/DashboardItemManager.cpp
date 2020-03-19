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
	selectItemWhenCreated = false;

	bgImage = addFileParameter("Backround Image", "Image to put in the background");
	bgImageScale = addFloatParameter("Background Image Scale", "The scale to multiply the image with", 1, .1f, 10);
	bgImageAlpha = addFloatParameter("Background Image Alpha", "The opacity of the background image", 1, 0, 1);
}

DashboardItemManager::~DashboardItemManager()
{
}