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

	canvasSize = addPoint2DParameter("Canvas Size", "If enabled, this will limit the canvas to this size", false);
	canvasSize->canBeDisabledByUser = true;
	canvasSize->setPoint(1200, 800);

	bgColor = addColorParameter("Backround Color", "Color of the background", Colours::transparentBlack);
	bgImage = addFileParameter("Backround Image", "Image to put in the background");
	bgImageScale = addFloatParameter("Background Image Scale", "The scale to multiply the image with", 1, .1f, 10);
	bgImageAlpha = addFloatParameter("Background Image Alpha", "The opacity of the background image", 1, 0, 1);
	
	setHasGridOptions(true);
}

DashboardItemManager::~DashboardItemManager()
{
}

void DashboardItemManager::clear()
{
	BaseManager::clear();
}

void DashboardItemManager::fillServerData(var &data)
{
	var iData;
	for (auto& i : items) iData.append(i->getServerData());
	data.getDynamicObject()->setProperty("items", iData);
}

void DashboardItemManager::onContainerParameterChanged(Parameter* p)
{
	notifyParameterFeedback(p);
}
