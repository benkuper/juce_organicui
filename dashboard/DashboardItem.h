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
	public Inspectable::InspectableListener
{
public:
	DashboardItem(Inspectable* _target = nullptr);
	virtual ~DashboardItem();

	virtual DashboardItemUI* createUI();
};