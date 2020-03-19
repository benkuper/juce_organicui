/*
  ==============================================================================

    DashboardManager.h
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardManager :
	public BaseManager<Dashboard>
{
public:
	juce_DeclareSingleton(DashboardManager, true);

	DashboardManager();
	~DashboardManager();

	BoolParameter* editMode;
	BoolParameter* snapping;
};
