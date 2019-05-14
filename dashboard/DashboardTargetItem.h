/*
  ==============================================================================

    DashboardTargetItem.h
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardTargetItem :
	public DashboardItem
{
public:
	DashboardTargetItem(TargetParameter::TargetType type = TargetParameter::TargetType::CONTROLLABLE);
	DashboardTargetItem(Controllable * controllable);
	DashboardTargetItem(ControllableContainer * container);
	virtual ~DashboardTargetItem();

	TargetParameter * target;
	DashboardItemTarget * getDashboardTarget();

	virtual void updateName();

	virtual void onContainerParameterChangedInternal(Parameter * p) override;

	virtual BaseItemMinimalUI<DashboardItem> * createUI() override;

	virtual String getTypeString() const override { return target->targetType == TargetParameter::CONTROLLABLE?"DashboardControllable":"DashboardControllableContainer" ; }

	static DashboardItem * create(var params) { return new DashboardTargetItem((TargetParameter::TargetType)(int)params.getProperty("targetType", TargetParameter::CONTROLLABLE));  }


};
