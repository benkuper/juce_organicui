/*
  ==============================================================================

    StringParameterUI.h
    Created: 9 Mar 2016 12:29:57am
    Author:  bkupe

  ==============================================================================
*/

#ifndef STRINGPARAMETERUI_H_INCLUDED
#define STRINGPARAMETERUI_H_INCLUDED



class StringParameterTextUI : 
	public ParameterUI, 
	public Label::Listener
{
public:
    StringParameterUI(Parameter * p);
    virtual ~StringParameterUI(){};

	StringParameter * stringParam; 
	
	Label nameLabel;
    Label valueLabel;

	float maxFontHeight;

	bool autoSize;
	void setAutoSize(bool value);

	bool isEditing();

	void setOpaqueBackground(bool value) override;

	virtual void feedbackStateChanged() override; 

	//void paint(Graphics &g) override;
    void resized() override;
	virtual void resizedInternal(juce::Rectangle<int> &r);

protected:
    void valueChanged(const var & v) override;
    virtual void labelTextChanged(Label * labelThatHasChanged) override;


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringParameterTextUI)

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
	ScopedPointer<ImageButton> relativeBT;

	void resizedInternal(juce::Rectangle<int> &r) override;

	void buttonClicked(Button * b) override;
};

class StringParameterTextUI :
	public StringParameterUI
{
public:
	StringParameterTextUI;
};

#endif  // STRINGPARAMETERUI_H_INCLUDED
