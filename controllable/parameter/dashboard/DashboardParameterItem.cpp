#include "DashboardParameterItem.h"

DashboardParameterItem::DashboardParameterItem(Parameter* parameter) :
	DashboardControllableItem(parameter),
	parameter(parameter),
	bgColor(nullptr), fgColor(nullptr), btImage(nullptr), style(nullptr)
{
	
	showValue = addBoolParameter("Show Value", "If checked, the value will be shown on the control", true);

	bgColor = addColorParameter("Background Color", "The color of the background", BG_COLOR, false);
	bgColor->canBeDisabledByUser = true;

	fgColor = addColorParameter("Foreground Color", "The foreground color if applicable", RED_COLOR, false);
	fgColor->canBeDisabledByUser = true;

	style = addEnumParameter("Style", "The style of this UI");
	style->addOption("Default", -1);

	if (parameter != nullptr && parameter->type == Controllable::FLOAT)
	{
		style->addOption("Horizontal Slider", 0)->addOption("Vertical Slider", 1)->addOption("Text", 2)->addOption("Time", 3);
	}

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

	style->clearOptions();

	style->addOption("Default", -1);
	if (parameter != nullptr && parameter->type == Controllable::FLOAT)
	{
		style->addOption("Horizontal Slider", 0)->addOption("Vertical Slider", 1)->addOption("Text", 2)->addOption("Time", 3);
	}
}
