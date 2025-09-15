/*
  ==============================================================================

    DashboardItem.h
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/


#pragma once
class DashboardItemUI;

class DashboardItem :
	public BaseItem,
	public Inspectable::InspectableListener,
	public DashboardFeedbackBroadcaster
{
public:
	DashboardItem(Inspectable* _target = nullptr, const juce::String &name = "Item");
	virtual ~DashboardItem();

	BoolParameter* isVisible;


	virtual juce::var getServerData();

	virtual void onContainerParameterChangedInternal(Parameter*) override;
	virtual void onControllableStateChanged(Controllable *) override;

	DECLARE_UI_FUNC;
};