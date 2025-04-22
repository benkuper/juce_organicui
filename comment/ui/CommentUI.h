/*
  ==============================================================================

    CommentUI.h
    Created: 4 Apr 2019 9:39:30am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class CommentUI :
	public ItemMinimalUI<CommentItem>,
	public juce::TextEditor::Listener
{
public:
	CommentUI(CommentItem * comment);
	~CommentUI();

	juce::TextEditor textUI;
	juce::ResizableBorderComponent resizer;

	void paint(juce::Graphics &g) override;
	void resized() override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
	void mouseDoubleClick(const juce::MouseEvent& e) override;

	void textEditorTextChanged(juce::TextEditor&) override;
	void textEditorEscapeKeyPressed(juce::TextEditor&) override;
	void textEditorReturnKeyPressed(juce::TextEditor&) override;
	void textEditorFocusLost(juce::TextEditor&) override;

	void focusLost(FocusChangeType type) override;

	bool canStartDrag(const juce::MouseEvent &e) override;

	void inspectableSelectionChanged(Inspectable* i) override;

	void disableTextEditor();

	void controllableFeedbackUpdateInternal(Controllable * c) override;
};
