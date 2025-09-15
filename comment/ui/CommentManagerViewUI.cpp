/*
  ==============================================================================

    CommentManagerViewUI.cpp
    Created: 4 Apr 2019 9:39:24am
    Author:  bkupe

  ==============================================================================
*/

CommentManagerViewUI::CommentManagerViewUI(CommentManager * manager) :
	ManagerViewUI(manager->niceName, manager)
{
	//animateItemOnAdd = false;
	setShowAddButton(false);
	bringToFrontOnSelect = false;
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

bool CommentManagerViewUI::keyPressed(const KeyPress &e)
{
	// skip manager handling (frame, home view)
	return ManagerUI<CommentManager, CommentItem>::keyPressed(e);
}

