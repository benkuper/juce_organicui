/*
  ==============================================================================

    DashboardManagerView.cpp
    Created: 19 Apr 2017 10:58:11pm
    Author:  Ben

  ==============================================================================
*/


DashboardManagerView::DashboardManagerView(const String &contentName, DashboardManager * manager) :
	ShapeShifterContentComponent(contentName),
	managerUI(manager),
	currentItemManagerUI(nullptr),
    currentDashboard(nullptr)
{
	contentIsFlexible = true; 
	
	addAndMakeVisible(&managerUI);
	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
	DashboardManager::getInstance()->addBaseManagerListener(this);
	
	helpID = "Dashboard";
}

DashboardManagerView::~DashboardManagerView()
{
	if(DashboardManager::getInstanceWithoutCreating() != nullptr) DashboardManager::getInstance()->removeBaseManagerListener(this);
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
}

void DashboardManagerView::setCurrentDashboard(Dashboard * d)
{
	if (currentDashboard == d) return;

	if (currentItemManagerUI != nullptr)
	{
		removeChildComponent(currentItemManagerUI.get());
		currentItemManagerUI = nullptr;
	}

	currentDashboard = d;

	if (currentDashboard != nullptr)
	{
		currentItemManagerUI.reset(new DashboardItemManagerUI(&currentDashboard->itemManager));
		addAndMakeVisible(currentItemManagerUI.get());
	}

	resized();
}

void DashboardManagerView::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	managerUI.setBounds(r.removeFromLeft(150));
	if (currentItemManagerUI != nullptr)
	{
		currentItemManagerUI->setBounds(r);
	}
}

void DashboardManagerView::inspectablesSelectionChanged()
{
	if (InspectableSelectionManager::mainSelectionManager->isEmpty()) return;
	Dashboard * cc = InspectableSelectionManager::mainSelectionManager->getInspectableAs<Dashboard>();
	if (cc == nullptr) return;
	setCurrentDashboard(cc);
}

void DashboardManagerView::itemRemoved(Dashboard * d)
{
	if (currentDashboard != nullptr && currentDashboard == d) setCurrentDashboard(nullptr);
}
