/*
  ==============================================================================

    CommentUI.h
    Created: 4 Apr 2019 9:39:30am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class CommentUI :
	public BaseItemMinimalUI<CommentItem>,
	public Label::Listener
{
public:
	CommentUI(CommentItem * comment);
	~CommentUI();

	Label textUI;

	void paint(Graphics &g) override;
	void resized() override;

	void labelTextChanged(Label *) override;
	
	bool canStartDrag(const MouseEvent &e) override;

	void controllableFeedbackUpdateInternal(Controllable * c) override;
};
