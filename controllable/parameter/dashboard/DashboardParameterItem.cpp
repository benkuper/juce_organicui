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

	if (parameter != nullptr)
	{
		parameter->removeParameterListener(this);
	}

	parameter = dynamic_cast<Parameter*>(i);

	if (parameter != nullptr)
	{
		parameter->addParameterListener(this);
	}


	style->clearOptions();
	style->addOption("Default", -1);
	if (parameter != nullptr && parameter->type == Controllable::FLOAT)
	{
		style->addOption("Horizontal Slider", 0)->addOption("Vertical Slider", 1)->addOption("Text", 2)->addOption("Time", 3);
	}
}

void DashboardParameterItem::onExternalParameterValueChanged(Parameter* p)
{
	DashboardControllableItem::onExternalParameterValueChanged(p);

	if (p == parameter)
	{
		var data(new DynamicObject());
		data.getDynamicObject()->setProperty("controlAddress", parameter->getControlAddress());
		data.getDynamicObject()->setProperty("value", parameter->value);
		notifyDataFeedback(data);
	}
}

var DashboardParameterItem::getServerData()
{
	var data = DashboardControllableItem::getServerData();
	if (controllable == nullptr || controllable.wasObjectDeleted()) return data;

	data.getDynamicObject()->setProperty("value", parameter->value);
	if (parameter->hasRange())
	{
		data.getDynamicObject()->setProperty("minVal", parameter->minimumValue);
		data.getDynamicObject()->setProperty("maxVal", parameter->maximumValue);
	}

	if (bgColor->enabled) data.getDynamicObject()->setProperty("bgColor", bgColor->value);
	if (fgColor->enabled) data.getDynamicObject()->setProperty("fgColor", bgColor->value);

	int styleValue = (int)style->getValueData();
	 data.getDynamicObject()->setProperty("style", styleValue);
	if (styleValue == -1)
	{
		if(parameter->type == Controllable::FLOAT) data.getDynamicObject()->setProperty("defaultUI", ((FloatParameter*)parameter.get())->defaultUI);
	}

	if (btImage->stringValue().isNotEmpty()) data.getDynamicObject()->setProperty("customImage", btImage->stringValue());


	return data;

}
