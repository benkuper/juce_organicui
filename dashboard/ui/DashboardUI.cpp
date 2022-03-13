/*
  ==============================================================================

	DashboardUI.cpp
	Created: 19 Apr 2017 11:06:43pm
	Author:  Ben

  ==============================================================================
*/

DashboardUI::DashboardUI(Dashboard* item) :
	BaseItemUI(item)
{
	setSize(100, 20);
	item->addAsyncDashboardListener(this);
}

DashboardUI::~DashboardUI()
{
	if(inspectable.wasObjectDeleted()) item->removeAsyncDashboardListener(this);
}

void DashboardUI::paint(Graphics& g)
{
	if (item == nullptr) return;
	bgColor = item->isBeingEdited ? DashboardManager::getInstance()->tabsSelectedBGColor->getColor() : NORMAL_COLOR;
	BaseItemUI::paint(g);
}

void DashboardUI::newMessage(const Dashboard::DashboardEvent& e)
{
	if (e.type == e.EDITING_UPDATE) repaint();
}
