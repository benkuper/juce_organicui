/*
  ==============================================================================

	StringParameterUI.h
	Created: 9 Mar 2016 12:29:57am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class StringParameterUI :
	public ParameterUI,
	public Label::Listener
{
public:
	StringParameterUI(Array<StringParameter*> parameters);
	virtual ~StringParameterUI();

	Array<StringParameter*> stringParams;
	StringParameter* stringParam;

	Label valueLabel;

	float maxFontHeight;

	bool autoSize;
	void setAutoSize(bool value);

	bool isEditing();

	virtual void updateTooltip() override;
	virtual void updateUIParamsInternal() override;

	void paint(Graphics& g) override;
	void resized() override;
	virtual void resizedInternal(juce::Rectangle<int>&) {} //to be overriden

protected:
	void valueChanged(const var& v) override;
	virtual void labelTextChanged(Label* labelThatHasChanged) override;


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterUI)

};


class StringParameterFileUI :
	public StringParameterUI,
	public Button::Listener,
	public ParameterListener
{
public:
	StringParameterFileUI(Array<StringParameter*> parameters);
	virtual ~StringParameterFileUI();

	Array<FileParameter*> fps;
	FileParameter* fp;
	TextButton browseBT;
	std::unique_ptr<ImageButton> explorerBT;
	
	static File lastSearchedFolder;

	virtual void addPopupMenuItemsInternal(PopupMenu* p) override;
	virtual void handleMenuSelectedID(int result) override;

	void resizedInternal(juce::Rectangle<int>& r) override;
	virtual void feedbackStateChanged() override;

	void buttonClicked(Button* b) override;
};



class StringParameterTextUI :
	public ParameterUI,
	public TextEditor::Listener
{
public:
	StringParameterTextUI(Array<StringParameter*> parameters);
	virtual ~StringParameterTextUI() {};

	Array<StringParameter*> stringParams;
	StringParameter* stringParam;

	TextEditor editor;
	virtual void feedbackStateChanged() override;

	//void paint(Graphics &g) override;
	void resized() override;

protected:
	void valueChanged(const var& v) override;
	virtual void textEditorTextChanged(TextEditor&) override;
	virtual void textEditorFocusLost(TextEditor&) override;
	virtual void textEditorReturnKeyPressed(TextEditor&) override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterTextUI)
};
