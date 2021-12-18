/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/
#include "DashboardCommentItem.h"

DashboardCommentItem::DashboardCommentItem() :
	DashboardItem(nullptr, "Comment")
{
	text = addStringParameter("Text", "The comment text", "I Love " + OrganicApplication::getInstance()->getApplicationName() + " !");
	text->multiline = true;

	size = addFloatParameter("Size", "The text size", 14, 0, 80);
	size->customUI = FloatParameter::LABEL;

	color = addColorParameter("Color", "The color of the text", TEXT_COLOR);
	bgAlpha = addFloatParameter("Background Alpha", "The alpha", 0, 0, 1);
}

DashboardCommentItem::~DashboardCommentItem()
{
}

var DashboardCommentItem::getServerData()
{
	var data = DashboardItem::getServerData();
	data.getDynamicObject()->setProperty("text", text->stringValue());
	data.getDynamicObject()->setProperty("size", size->floatValue());
	data.getDynamicObject()->setProperty("color", color->value);
	data.getDynamicObject()->setProperty("backgroundAlpha", bgAlpha->floatValue());
	return data;
}

DashboardItemUI * DashboardCommentItem::createUI()
{
	return new DashboardCommentItemUI(this);
}

var DashboardCommentItem::getItemParameterFeedback(Parameter* p)
{
	var data = DashboardItem::getItemParameterFeedback(p);
	data.getDynamicObject()->setProperty("targetType", this->getTypeString());

	return data;
}