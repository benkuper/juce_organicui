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
	public juce::Label::Listener
{
public:
	StringParameterUI(juce::Array<StringParameter*> parameters);
	virtual ~StringParameterUI();

	juce::Array<StringParameter*> stringParams;
	StringParameter* stringParam;

	juce::Label valueLabel;

	float maxFontHeight;

	bool autoSize;
	void setAutoSize(bool value);

	bool isEditing();

	virtual void updateTooltip() override;
	virtual void updateUIParamsInternal() override;

	void paint(juce::Graphics& g) override;
	void resized() override;
	virtual void resizedInternal(juce::Rectangle<int>&) {} //to be overriden

protected:
	void valueChanged(const juce::var& v) override;
	virtual void labelTextChanged(juce::Label* labelThatHasChanged) override;


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterUI)

};


class StringParameterFileUI :
	public StringParameterUI,
	public juce::Button::Listener,
	public ParameterListener
{
public:
	StringParameterFileUI(juce::Array<StringParameter*> parameters);
	virtual ~StringParameterFileUI();

	juce::Array<FileParameter*> fps;
	FileParameter* fp;
	juce::TextButton browseBT;
	std::unique_ptr<juce::ImageButton> explorerBT;
	
	static juce::File lastSearchedFolder;

	virtual void addPopupMenuItemsInternal(juce::PopupMenu* p) override;
	virtual void handleMenuSelectedID(int result) override;

	void resizedInternal(juce::Rectangle<int>& r) override;
	virtual void feedbackStateChanged() override;

	void buttonClicked(juce::Button* b) override;
};



class StringParameterTextUI :
	public ParameterUI,
	public juce::TextEditor::Listener
{
public:
	StringParameterTextUI(juce::Array<StringParameter*> parameters);
	virtual ~StringParameterTextUI() {};

	juce::Array<StringParameter*> stringParams;
	StringParameter* stringParam;

	juce::TextEditor editor;
	virtual void feedbackStateChanged() override;

	//void paint(Graphics &g) override;
	void resized() override;

protected:
	void valueChanged(const juce::var& v) override;
	virtual void textEditorTextChanged(juce::TextEditor&) override;
	virtual void textEditorFocusLost(juce::TextEditor&) override;
	virtual void textEditorReturnKeyPressed(juce::TextEditor&) override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterTextUI)
};
