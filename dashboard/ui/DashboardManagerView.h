/*
  ==============================================================================

    DashboardManagerView.h
    Created: 19 Apr 2017 10:58:11pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardManagerView :
	public ShapeShifterContentComponent,
	public InspectableSelectionManager::Listener,
	public DashboardManager::ManagerListener
{
public:
	DashboardManagerView(const String &contentName, DashboardManager * manager);
	~DashboardManagerView();

	DashboardManagerUI managerUI;
	std::unique_ptr<DashboardItemManagerUI> currentItemManagerUI;

	Dashboard * currentDashboard;
	void setCurrentDashboard(Dashboard *);

    void resized() override;

	void inspectablesSelectionChanged() override;

	void itemRemoved(Dashboard *) override;

	static DashboardManagerView * create(const String &contentName) { return new DashboardManagerView(contentName, DashboardManager::getInstance()); }
};