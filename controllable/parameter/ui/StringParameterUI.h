/*
  ==============================================================================

    StringParameterUI.h
    Created: 9 Mar 2016 12:29:57am
    Author:  bkupe

  ==============================================================================
*/

#ifndef STRINGPARAMETERUI_H_INCLUDED
#define STRINGPARAMETERUI_H_INCLUDED



class StringParameterUI : 
	public ParameterUI, 
	public Label::Listener
{
public:
    StringParameterUI(Parameter * p);
    virtual ~StringParameterUI(){};

    Label nameLabel;
    Label valueLabel;

	String prefix;
	String suffix;

	float maxFontHeight;

	bool autoSize;
	void setAutoSize(bool value);

	bool isEditing();

	void setPrefix(const String &_prefix);
	void setSuffix(const String &_suffix);

	void setOpaqueBackground(bool value) override;

	virtual void setForceFeedbackOnlyInternal() override;

	//void paint(Graphics &g) override;
    void resized() override;
	virtual void resizedInternal(Rectangle<int> &r);

protected:
    void valueChanged(const var & v) override;
    virtual void labelTextChanged(Label * labelThatHasChanged) override;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterUI)

};


class StringParameterFileUI :
	public StringParameterUI,
	public Button::Listener
{
public:
	StringParameterFileUI(Parameter * p);
	virtual ~StringParameterFileUI();

	TextButton browseBT;
	void resizedInternal(Rectangle<int> &r) override;

	void buttonClicked(Button * b) override;
};

#endif  // STRINGPARAMETERUI_H_INCLUDED
