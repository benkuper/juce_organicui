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
    StringParameterUI(Parameter * p);
    virtual ~StringParameterUI();

	StringParameter * stringParam; 
	
    Label valueLabel;

	float maxFontHeight;

	bool autoSize;
	void setAutoSize(bool value);

	bool isEditing();

	void setOpaqueBackground(bool value) override;

	virtual void feedbackStateChanged() override; 

	virtual void updateTooltip() override;

	void paint(Graphics &g) override;
    void resized() override;
	virtual void resizedInternal(juce::Rectangle<int> &) {} //to be overriden

protected:
    void valueChanged(const var & v) override;
    virtual void labelTextChanged(Label * labelThatHasChanged) override;


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterUI)

};


class StringParameterFileUI :
	public StringParameterUI,
	public Button::Listener,
	public ParameterListener
{
public:
	StringParameterFileUI(Parameter * p);
	virtual ~StringParameterFileUI();

	FileParameter * fp;
	TextButton browseBT;
	std::unique_ptr<ImageButton> relativeBT;

	void resizedInternal(juce::Rectangle<int> &r) override;
	virtual void feedbackStateChanged() override;

	void buttonClicked(Button * b) override;
};



class StringParameterTextUI :
	public ParameterUI,
	public TextEditor::Listener
{
public:
	StringParameterTextUI(Parameter * p);
	virtual ~StringParameterTextUI() {};

	StringParameter * stringParam;

	TextEditor editor;
	virtual void feedbackStateChanged() override;

	//void paint(Graphics &g) override;
	void resized() override;

protected:
	void valueChanged(const var & v) override;
	virtual void textEditorTextChanged(TextEditor &) override;
	virtual void textEditorFocusLost(TextEditor &) override;
	virtual void textEditorReturnKeyPressed(TextEditor &) override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterTextUI)
};
