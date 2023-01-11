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
	public TextEditor::Listener
{
public:
	CommentUI(CommentItem * comment);
	~CommentUI();

	TextEditor textUI;
	ResizableBorderComponent resizer;

	void paint(Graphics &g) override;
	void resized() override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
	void mouseDoubleClick(const MouseEvent& e) override;

	void textEditorTextChanged(TextEditor&) override;
	void textEditorEscapeKeyPressed(TextEditor&) override;
	void textEditorReturnKeyPressed(TextEditor&) override;
	void textEditorFocusLost(TextEditor&) override;

	void focusLost(FocusChangeType type) override;

	bool canStartDrag(const MouseEvent &e) override;

	void inspectableSelectionChanged(Inspectable* i) override;

	void disableTextEditor();

	void controllableFeedbackUpdateInternal(Controllable * c) override;
};
