/*
  ==============================================================================

    DashboardItem.h
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/


#pragma once

class DashboardControllableItem :
	public DashboardItem
{
public:
	DashboardControllableItem(Controllable * controllable = nullptr);
	~DashboardControllableItem();


	virtual void setTarget(Inspectable * target) override;
	void updateName();


	DashboardItemUI * createUI() override;

	String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "DashboardControllableItem"; }

	static DashboardControllableItem * create(var params) { return new DashboardControllableItem(); }
};