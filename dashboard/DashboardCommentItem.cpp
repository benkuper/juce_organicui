/*
  ==============================================================================

    DashboardItem.cpp
    Created: 19 Apr 2017 11:06:51pm
    Author:  Ben

  ==============================================================================
*/

DashboardCommentItem::DashboardCommentItem() :
	DashboardItem(nullptr, "Comment")
{
	text = addStringParameter("Text", "The comment text", "I Love " + OrganicApplication::getInstance()->getApplicationName() + " !");
	text->multiline = true;

	size = addFloatParameter("Size", "The text size", 14, 0, 80);
	size->customUI = FloatParameter::LABEL;

	setHasCustomColor(true);
	itemColor->setDefaultValue(TEXT_COLOR);

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
	data.getDynamicObject()->setProperty("color", itemColor->value);
	data.getDynamicObject()->setProperty("backgroundAlpha", bgAlpha->floatValue());
	return data;
}

var DashboardCommentItem::getItemParameterFeedback(Parameter* p)
{
	var data = DashboardItem::getItemParameterFeedback(p);
	data.getDynamicObject()->setProperty("targetType", this->getTypeString());

	return data;
}

IMPLEMENT_UI_FUNC(DashboardCommentItem);