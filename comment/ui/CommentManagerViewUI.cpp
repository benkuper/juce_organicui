#include "CommentManagerViewUI.h"
/*
  ==============================================================================

    CommentManagerViewUI.cpp
    Created: 4 Apr 2019 9:39:24am
    Author:  bkupe

  ==============================================================================
*/

CommentManagerViewUI::CommentManagerViewUI(CommentManager * manager) :
	BaseManagerViewUI(manager->niceName, manager)
{
	autoFilterHitTestOnItems = true;
	validateHitTestOnNoItem = false;
	transparentBG = true;
}

CommentManagerViewUI::~CommentManagerViewUI()
{
}

void CommentManagerViewUI::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& d)
{
	//skip manager handling

	Component::mouseWheelMove(e, d);
}
