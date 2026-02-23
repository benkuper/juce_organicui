#include "JuceHeader.h"


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
	btImage->canBeDisabledByUser = true;
	btImage->setEnabled(false);

	setInspectable(parameter);
	ghostInspectable();
}

DashboardParameterItem::~DashboardParameterItem()
{
	if (parameter != nullptr && !parameter.wasObjectDeleted())
	{
		parameter->removeParameterListener(this);
	}
}

DashboardItemUI* DashboardParameterItem::createUI()
{
	return new DashboardParameterItemUI(this);
}

InspectableEditor* DashboardParameterItem::getStyleEditor(bool isRoot, Array<Inspectable*> c)
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
		target->typesFilter.clear();
		target->typesFilter.add(parameter->getTypeString());
	}

	updateStyleOptions();
}

void DashboardParameterItem::onExternalParameterValueChanged(Parameter* p)
{
	DashboardControllableItem::onExternalParameterValueChanged(p);
		
	if (isClearing) return;

	if (p == parameter)
	{
		var data(new DynamicObject());
		data.getDynamicObject()->setProperty("feedbackType", "targetFeedback");
		data.getDynamicObject()->setProperty("controlAddress", parameter->getControlAddress());
		data.getDynamicObject()->setProperty("type", parameter->getTypeString());

		var val = var();

		if (parameter->getTypeString() == FileParameter::getTypeStringStatic()) val = parameter->getControlAddress();
		else if (parameter->type == Controllable::ENUM) val = ((EnumParameter*)parameter.get())->getValueKey();
		else val = parameter->getValue().clone();

		data.getDynamicObject()->setProperty("value", val);

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
			if (parameter->type == Controllable::ENUM) style->addOption("Horizontal Bar", 20)->addOption("Vertical Bar", 21);
			if (parameter->type == Controllable::BOOL) style->addOption("Toggle Checkbox", 22)->addOption("Toggle Button", 23)->addOption("Momentary Checkbox", 24)->addOption("Momentary Button", 25);
			style->addOption("Color Circle", 10)->addOption("Color Rectangle", 11);
			style->customGetEditorFunc = std::bind(&DashboardParameterItem::getStyleEditor, this, std::placeholders::_1, std::placeholders::_2);
		}
		else if (parameter->type == Controllable::POINT2D)
		{
			style->addOption("2D Canvas", 12);
		}

		if (!style->isOverriden)
		{
			EnumParameter* ep = nullptr;
			switch (parameter->type)
			{
			case Controllable::BOOL: ep = ProjectSettings::getInstance()->boolDefaultStyle; break;
			case Controllable::FLOAT: ep = ProjectSettings::getInstance()->floatDefaultStyle; break;
			case Controllable::INT: ep = ProjectSettings::getInstance()->intDefaultStyle; break;
			case Controllable::ENUM: ep = ProjectSettings::getInstance()->enumDefaultStyle; break;
			default:
				break;
			}

			if (ep != nullptr && ep->enabled) val = ep->getValueData();
		}
		else
		{
			//viewUISize->setPoint(100, 20);
		}
	}

	if (!val.isVoid()) style->setValueWithData(val);
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
	data.getDynamicObject()->setProperty("showValue", showValue->boolValue());

	int styleValue = (int)style->getValueData();
	data.getDynamicObject()->setProperty("style", styleValue);
	switch (styleValue)
	{
	case -1:
		if (parameter->type == Controllable::FLOAT) data.getDynamicObject()->setProperty("defaultUI", ((FloatParameter*)parameter.get())->defaultUI);
		break;

	case 10:
	case 11:
	{
		var colorMapData(new DynamicObject());
		HashMap<var, Colour>::Iterator it(parameter->colorStatusMap);
		while (it.next())
		{
			Colour c = it.getValue();
			var cData;
			cData.append(c.getRed());
			cData.append(c.getGreen());
			cData.append(c.getBlue());
			cData.append(c.getAlpha());
			colorMapData.getDynamicObject()->setProperty(it.getKey().toString(), cData);
		}
		data.getDynamicObject()->setProperty("colorMap", colorMapData);
	}
	break;

	case 22:
	case 23:
	case 24:
	case 25:
		data.getDynamicObject()->setProperty("momentaryMode", styleValue == 24 || styleValue == 25);
		data.getDynamicObject()->setProperty("buttonUI", styleValue == 23 || styleValue == 25);
		break;

	}

	if (btImage->enabled && btImage->stringValue().isNotEmpty()) data.getDynamicObject()->setProperty("customImage", btImage->getControlAddress());


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


	default:
		break;
	}

	return data;
}

DashboardTargetParameterItem::DashboardTargetParameterItem(TargetParameter* parameter) :
	DashboardParameterItem(parameter)
{
	showFullAddress = addBoolParameter("Show Full Address", "If checked, this will force show the full address of the target", false, false);
	showParentName = addBoolParameter("Show Parent Name", "If checked, this will force show the parent name of the target", false, false);
	parentLabelLevel = addIntParameter("Parent Label Level", "If checked, this will force search through X level of parents", 2, 1, 20, false);
	showLearnButton = addBoolParameter("Show Learn Button", "If checked, show learn function", false, false);

	showFullAddress->canBeDisabledByUser = true;
	showParentName->canBeDisabledByUser = true;
	parentLabelLevel->canBeDisabledByUser = true;
	showLearnButton->canBeDisabledByUser = true;
}

DashboardTargetParameterItem::~DashboardTargetParameterItem()
{
}

var DashboardTargetParameterItem::getServerData()
{
	var data = DashboardParameterItem::getServerData();
	if (showFullAddress->enabled) data.getDynamicObject()->setProperty("showFullAddress", showFullAddress->boolValue());
	if (showParentName->enabled) data.getDynamicObject()->setProperty("showParentName", showParentName->boolValue());
	if (parentLabelLevel->enabled) data.getDynamicObject()->setProperty("parentLabelLevel", parentLabelLevel->intValue());
	if (showLearnButton->enabled) data.getDynamicObject()->setProperty("showLearnButton", showLearnButton->intValue());

	return data;
}

DashboardEnumParameterItem::DashboardEnumParameterItem(EnumParameter* parameter) :
	DashboardParameterItem(parameter)
{
	if (parameter == nullptr) return;
	((EnumParameter*)parameter)->addEnumParameterListener(this);
}

DashboardEnumParameterItem::~DashboardEnumParameterItem()
{
	if (parameter == nullptr || parameter.wasObjectDeleted()) return;

	((EnumParameter*)parameter.get())->removeEnumParameterListener(this);

}

void DashboardEnumParameterItem::enumOptionAdded(EnumParameter*, const String&)
{
	notifyParameterFeedback(parameter);
}

void DashboardEnumParameterItem::enumOptionUpdated(EnumParameter*, int index, const String& prevKey, const String& newKey)
{
	notifyParameterFeedback(parameter);
}

void DashboardEnumParameterItem::enumOptionRemoved(EnumParameter*, const String&)
{
	notifyParameterFeedback(parameter);
}
