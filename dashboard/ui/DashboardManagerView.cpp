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
	currentDashboard(nullptr)
{
	contentIsFlexible = true; 
	
	addAndMakeVisible(&managerUI);
	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
	DashboardManager::getInstance()->addBaseManagerListener(this);
	
}

DashboardManagerView::~DashboardManagerView()
{
	if(DashboardManager::getInstanceWithoutCreating() != nullptr) DashboardManager::getInstance()->removeBaseManagerListener(this);
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
}

void DashboardManagerView::setCurrentDashboard(Dashboard * d)
{
	if (currentDashboard == d) return;

	if (view != nullptr)
	{
		removeChildComponent(view);
		view = nullptr;
	}

	currentDashboard = d;

	if (currentDashboard != nullptr)
	{
		view = new DashboardView(&currentDashboard->panelManager);
		addAndMakeVisible(view);
	}

	resized();
}

void DashboardManagerView::resized()
{
	Rectangle<int> r = getLocalBounds();
	managerUI.setBounds(r.removeFromLeft(100));
	if (view != nullptr)
	{
		view->setBounds(r);
	}
}

void DashboardManagerView::inspectablesSelectionChanged()
{
	if (InspectableSelectionManager::mainSelectionManager->isEmpty()) return;
	Dashboard * cc = dynamic_cast<Dashboard *>(InspectableSelectionManager::mainSelectionManager->currentInspectables[0]);
	if (cc == nullptr) return;
	setCurrentDashboard(cc);
}

void DashboardManagerView::itemRemoved(Dashboard * d)
{
	if (currentDashboard != nullptr && currentDashboard == d) setCurrentDashboard(nullptr);
}
