DashboardParameterItem::DashboardParameterItem(Parameter* parameter) :
	DashboardControllableItem(parameter),
	parameter(nullptr),
	bgColor(nullptr), fgColor(nullptr), btImage(nullptr), style(nullptr)
{

	showValue = addBoolParameter("Show Value", "If checked, the value will be shown on the control", true);

	bgColor = addColorParameter("Background Color", "The color of the background", BG_COLOR, false);
	bgColor->canBeDisabledByUser = true;

	fgColor = addColorParameter("Foreground Color", "The foreground color if applicable", RED_COLOR, false);
	fgColor->canBeDisabledByUser = true;

	style = addEnumParameter("Style", "The style of this UI");

	btImage = addFileParameter("Toggle image", "The image of the toggle");

	setInspectable(parameter);
	ghostInspectable();
}

DashboardParameterItem::~DashboardParameterItem()
{
}

DashboardItemUI* DashboardParameterItem::createUI()
{
	return new DashboardParameterItemUI(this);
}

InspectableEditor* DashboardParameterItem::getStyleEditor(Inspectable* c, bool isRoot)
{
	return new DashboardParameterStyleEditor(style, this, isRoot);
}

void DashboardParameterItem::setInspectableInternal(Inspectable* i)
{
	DashboardControllableItem::setInspectableInternal(i);

	if (parameter == i) return;

	if (parameter != nullptr)
	{
		parameter->removeParameterListener(this);
	}

	parameter = dynamic_cast<Parameter*>(i);

	if (parameter != nullptr)
	{
		parameter->addParameterListener(this);
	}

	updateStyleOptions();
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

void DashboardParameterItem::updateStyleOptions()
{
	var val = style->getValueData();
	style->clearOptions();
	style->addOption("Default", -1);

	if (parameter != nullptr)
	{
		if (parameter->type == Controllable::BOOL || parameter->type == Controllable::FLOAT || parameter->type == Controllable::INT || parameter->type == Controllable::ENUM)
		{
			if (parameter->type != Controllable::ENUM && parameter->type != Controllable::BOOL) style->addOption("Horizontal Slider", 0)->addOption("Vertical Slider", 1)->addOption("Rotary Slider", 5)->addOption("Text", 2)->addOption("Time", 3);
			style->addOption("Color Circle", 10)->addOption("Color Square", 11);
			style->customGetEditorFunc = std::bind(&DashboardParameterItem::getStyleEditor, this, std::placeholders::_1, std::placeholders::_2);
		}
		else if (parameter->type == Controllable::POINT2D)
		{
			style->addOption("2D Canvas", 12);
		}
	}
	
	if(!val.isVoid()) style->setValueWithData(val);
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
	if (fgColor->enabled) data.getDynamicObject()->setProperty("fgColor", fgColor->value);

	int styleValue = (int)style->getValueData();
	data.getDynamicObject()->setProperty("style", styleValue);
	if (styleValue == -1)
	{
		if (parameter->type == Controllable::FLOAT) data.getDynamicObject()->setProperty("defaultUI", ((FloatParameter*)parameter.get())->defaultUI);
	}

	if (btImage->stringValue().isNotEmpty()) data.getDynamicObject()->setProperty("customImage", btImage->getControlAddress());


	switch (parameter->type)
	{
	case Parameter::ENUM:
	{
		var epOptions;
		EnumParameter* ep = (EnumParameter*)parameter.get();
		for (auto& ev : ep->enumValues)
		{
			var epData(new DynamicObject());
			epData.getDynamicObject()->setProperty("key", ev->key);
			epData.getDynamicObject()->setProperty("id", ev->value);
			epOptions.append(epData);
		}

		data.getDynamicObject()->setProperty("options", epOptions);
	}
	break;
	case Parameter::POINT2D:
	{
		Point2DParameter* p2d = (Point2DParameter*)parameter.get();
		data.getDynamicObject()->setProperty("stretchMode", p2d->extendedEditorStretchMode);
		data.getDynamicObject()->setProperty("invertX", p2d->extendedEditorInvertX);
		data.getDynamicObject()->setProperty("invertY", p2d->extendedEditorInvertY);
	}
	break;
	}
	
	return data;
}
