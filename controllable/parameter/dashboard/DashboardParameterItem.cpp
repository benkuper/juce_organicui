#include "DashboardParameterItem.h"

DashboardParameterItem::DashboardParameterItem(Parameter* parameter) :
	DashboardControllableItem(parameter),
	parameter(parameter)
{
}

DashboardParameterItem::~DashboardParameterItem()
{
}

DashboardItemUI* DashboardParameterItem::createUI()
{
	return new DashboardParameterItemUI(this);
}

void DashboardParameterItem::setInspectableInternal(Inspectable* i)
{
	DashboardControllableItem::setInspectableInternal(i);
	parameter = dynamic_cast<Parameter*>(i);
}
