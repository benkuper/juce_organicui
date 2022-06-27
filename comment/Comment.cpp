/*
  ==============================================================================

    Comment.cpp
    Created: 4 Apr 2019 9:39:09am
    Author:  bkupe

  ==============================================================================
*/

CommentItem::CommentItem() :
	BaseItem("Comment", false)
{
	itemDataType = "Comment";
	text = addStringParameter("Text", "The comment text", "I Love "+ OrganicApplication::getInstance()->getApplicationName()+ " !");
	text->multiline = true;

	setHasCustomColor(true);
	itemColor->hideInEditor = false;
	itemColor->setDefaultValue(TEXT_COLOR);

	size = addFloatParameter("Size", "The text size", 14, 0,80);
	size->customUI = FloatParameter::LABEL;


	bgAlpha = addFloatParameter("Background Alpha", "The alpha", 0, 0, 1);

	viewUISize->setPoint(140, 30);
}

CommentItem::~CommentItem()
{
}
