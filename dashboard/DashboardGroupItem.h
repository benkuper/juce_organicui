/*
  ==============================================================================

    DashboardItem.h
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardGroupItem :
	public BaseItem,
	public Inspectable::InspectableListener
{
public:
	DashboardGroupItem(Inspectable * _target = nullptr);
	virtual ~DashboardGroupItem();


	virtual BaseItemMinimalUI<DashboardItem> * createUI();
};