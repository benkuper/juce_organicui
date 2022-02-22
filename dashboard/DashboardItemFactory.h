/*
  ==============================================================================

    DashboardItemFactory.h
    Created: 19 Apr 2019 11:06:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemFactory :
	public Factory<DashboardItem>
{
public:
	juce_DeclareSingleton(DashboardItemFactory, true)

	DashboardItemFactory();
	virtual ~DashboardItemFactory() {}

	Array<DashboardItemProvider *> providers;
	Array<DashboardItemProvider *> specialProviders;

	void buildPopupMenu() override;
	void showCreateMenu(std::function<void(DashboardItem *)> returnFunc) override;
	DashboardItem * createFromMenuResult(int result) override;
};
