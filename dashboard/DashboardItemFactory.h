/*
  ==============================================================================

	DashboardItemFactory.h
	Created: 19 Apr 2019 11:06:56pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemFactory :
	public Factory
{
public:
	juce_DeclareSingleton(DashboardItemFactory, true);

	DashboardItemFactory();
	virtual ~DashboardItemFactory() {}

	juce::Array<DashboardItemProvider*> providers;
	juce::Array<DashboardItemProvider*> specialProviders;

	void buildPopupMenu(int startOffset = 0) override;
	void showCreateMenu(std::function<void(BaseItem*)> returnFunc) override;
	BaseItem* createFromMenuResult(int result) override;
};
