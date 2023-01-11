/*
  ==============================================================================

    CommentManagerViewUI.h
    Created: 4 Apr 2019 9:39:24am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class CommentManagerViewUI :
	public BaseManagerViewUI<CommentManager, CommentItem, CommentUI>
{
public:
	CommentManagerViewUI(CommentManager * manager);
	~CommentManagerViewUI();

	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& d) override;
	bool keyPressed(const KeyPress &e) override;

	bool isInterestedInDragSource(const SourceDetails& e) { return false; } //handled by containing manager
};
