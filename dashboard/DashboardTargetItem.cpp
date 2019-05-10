/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardTargetItem::DashboardTargetItem(TargetParameter::TargetType type)
{
	nameCanBeChangedByUser = false;
	target = addTargetParameter("Target", "The target to show the UI");
	target->targetType = type;
}

DashboardTargetItem::DashboardTargetItem(Controllable * controllable) :
	DashboardTargetItem(TargetParameter::CONTROLLABLE)
{
	target->setValueFromTarget(controllable);
}

DashboardTargetItem::DashboardTargetItem(ControllableContainer * container) :
	DashboardTargetItem(TargetParameter::CONTAINER)
{
	target->setValueFromTarget(container);
}

DashboardTargetItem::~DashboardTargetItem()
{
}

DashboardItemTarget * DashboardTargetItem::getDashboardTarget()
{
	if (target->targetType == TargetParameter::CONTROLLABLE) return dynamic_cast<DashboardItemTarget *>(target->target.get());
	else return dynamic_cast<DashboardItemTarget *>(target->targetContainer.get());
}

void DashboardTargetItem::updateName()
{
	if(target->targetType == TargetParameter::CONTROLLABLE)
	{
		if(target->target != nullptr) setNiceName((target->target->parentContainer != nullptr ? target->target->parentContainer->niceName + " : " : "") + target->target->niceName);
	}
	else
	{
		if(target->targetContainer != nullptr) setNiceName((target->targetContainer->parentContainer != nullptr ? target->targetContainer->parentContainer->niceName + " : " : "") + target->targetContainer->niceName);
	}
	
}

void DashboardTargetItem::onContainerParameterChangedInternal(Parameter * p)
{
	DashboardItem::onContainerParameterChangedInternal(p);
	if (p == target) updateName();
}

BaseItemMinimalUI<DashboardItem> * DashboardTargetItem::createUI()
{
	return new DashboardTargetItemUI(this);
}
