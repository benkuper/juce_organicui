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
	public juce::TextEditor::Listener
{
public:
	DashboardCommentItemUI(DashboardCommentItem * comment);
	~DashboardCommentItemUI();

	DashboardCommentItem* comment;

	juce::TextEditor textUI;
	
	void resizedDashboardItemInternal() override;

	void updateEditModeInternal(bool editMode) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseDoubleClick(const juce::MouseEvent& e) override;

	void textEditorTextChanged(juce::TextEditor&) override;
	void textEditorEscapeKeyPressed(juce::TextEditor&) override;
	void textEditorReturnKeyPressed(juce::TextEditor&) override;
	void textEditorFocusLost(juce::TextEditor&) override;

	void focusLost(FocusChangeType type) override;

	bool canStartDrag(const juce::MouseEvent& e) override;

	void inspectableSelectionChanged(Inspectable* i) override;

	void disableTextEditor();

	void controllableFeedbackUpdateInternal(Controllable* c) override;
};