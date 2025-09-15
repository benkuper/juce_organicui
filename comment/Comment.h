/*
  ==============================================================================

    Comment.h
    Created: 4 Apr 2019 9:39:09am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class CommentItem :
	public BaseItem
{
public:
	CommentItem();
	~CommentItem();

	StringParameter * text;
	FloatParameter * size;
	FloatParameter * bgAlpha;

	DECLARE_TYPE("Comment");

	DECLARE_UI_FUNC;
};
