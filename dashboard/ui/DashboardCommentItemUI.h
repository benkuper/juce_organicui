/*
  ==============================================================================

    DashboardUI.h
    Created: 19 Apr 2017 11:06:43pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardCommentItemUI :
	public DashboardItemUI,
	public TextEditor::Listener
{
public:
	DashboardCommentItemUI(DashboardCommentItem * comment);
	~DashboardCommentItemUI();

	DashboardCommentItem* comment;

	TextEditor textUI;
	
	void resizedDashboardItemInternal() override;

	void updateEditModeInternal(bool editMode) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseDoubleClick(const MouseEvent& e) override;

	void textEditorTextChanged(TextEditor&) override;
	void textEditorEscapeKeyPressed(TextEditor&) override;
	void textEditorReturnKeyPressed(TextEditor&) override;
	void textEditorFocusLost(TextEditor&) override;

	void focusLost(FocusChangeType type) override;

	bool canStartDrag(const MouseEvent& e) override;

	void inspectableSelectionChanged(Inspectable* i) override;

	void disableTextEditor();

	void controllableFeedbackUpdateInternal(Controllable* c) override;
};