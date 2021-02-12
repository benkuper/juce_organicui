/*
  ==============================================================================

    Comment.cpp
    Created: 4 Apr 2019 9:39:09am
    Author:  bkupe

  ==============================================================================
*/

CommentItem::CommentItem() :
	BaseItem("Comment",false)
{
	itemDataType = "Comment";
	text = addStringParameter("Text", "The comment text", "I Love "+ OrganicApplication::getInstance()->getApplicationName()+ " !");
	text->multiline = true;

	size = addFloatParameter("Size", "The text size", 14, 0,80);
	size->customUI = FloatParameter::LABEL;

	color = addColorParameter("Color", "The color of the text", TEXT_COLOR);
	bgAlpha = addFloatParameter("Background Alpha", "The alpha", 0, 0, 1);
}

CommentItem::~CommentItem()
{
}
