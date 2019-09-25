#include "DashboardParameterItem.h"

DashboardParameterItem::DashboardParameterItem(Parameter* parameter) :
	DashboardControllableItem(parameter),
	parameter(parameter),
	bgColor(nullptr), fgColor(nullptr), btImage(nullptr)
{
	showValue = addBoolParameter("Show Value", "If checked, the value will be shown on the control", true);

	bgColor = addColorParameter("Background Color", "The color of the background", BG_COLOR, false);
	bgColor->canBeDisabledByUser = true;

	fgColor = addColorParameter("Bar Color", "The color of the slider's bar", RED_COLOR, false);
	fgColor->canBeDisabledByUser = true;

	btImage = addFileParameter("Toggle image", "The image of the toggle");
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
