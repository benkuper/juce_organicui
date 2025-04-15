/*
  ==============================================================================

    CommentManagerViewUI.h
    Created: 4 Apr 2019 9:39:24am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class CommentManagerViewUI :
	public ManagerViewUI<CommentManager, CommentItem, CommentUI>
{
public:
	CommentManagerViewUI(CommentManager * manager);
	~CommentManagerViewUI();

	void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d) override;
	bool keyPressed(const juce::KeyPress &e) override;

	bool isInterestedInDragSource(const SourceDetails& e) override { return false; } //handled by containing manager
};
