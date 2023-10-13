/*
  ==============================================================================

    EnumParameterUI.h
    Created: 29 Sep 2016 5:35:12pm
    Author:  bkupe

  ==============================================================================
*/


#pragma once

class EnumOptionManager :
	public juce::Component,
	public juce::Label::Listener
{
public:
	EnumOptionManager(EnumParameter* ep);
	~EnumOptionManager();

	EnumParameter* ep;
	juce::Viewport viewport;
	juce::Component container;

	class EnumOptionUI :
		public juce::Component
	{
	public:
		EnumOptionUI(EnumParameter* ui, int index);

		EnumParameter* ep;
		int index;

		juce::Label keyLabel;
		juce::Label valueLabel;

		void resized() override;
	};

	juce::OwnedArray<EnumOptionUI> optionsUI;

	void paint(juce::Graphics& g) override;
	void resized() override;
	void labelTextChanged(juce::Label* l) override;
};

class EnumParameterUIBase :
	public ParameterUI,
	public EnumParameter::AsyncListener
{
public:
	EnumParameterUIBase(juce::Array<EnumParameter*> parameters);
	virtual ~EnumParameterUIBase();

	juce::Array<EnumParameter*> eps;
	EnumParameter* ep;

	virtual void updateFromParameter() {}

	virtual void addPopupMenuItemsInternal(juce::PopupMenu* p) override;
	virtual void handleMenuSelectedID(int result) override;

	void newMessage(const EnumParameter::EnumParameterEvent& e) override;
};

class EnumParameterUI :
	public EnumParameterUIBase,
	public juce::ComboBox::Listener
{
public:
    EnumParameterUI(juce::Array<EnumParameter *> parameters);
    virtual ~EnumParameterUI();

	juce::ComboBox cb;

	void updateFromParameter() override;
	juce::String getSelectedKey();

	juce::var prevValue;

	void resized() override;

	// Inherited via Listener
	virtual void comboBoxChanged(juce::ComboBox *) override;


private:
	juce::HashMap<int, juce::String> idKeyMap;
	juce::HashMap<juce::String,int> keyIdMap;

protected:
	void updateUIParamsInternal() override;
    void valueChanged(const juce::var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnumParameterUI)
};



class EnumParameterButtonBarUI :
	public EnumParameterUIBase,
	public juce::TextButton::Listener
{
public:
	EnumParameterButtonBarUI(juce::Array<EnumParameter*> parameters);
	~EnumParameterButtonBarUI();

	juce::OwnedArray<juce::TextButton> buttons;
	bool isVertical;

	void updateFromParameter() override;
	void resized() override;

protected:
	void updateUIParamsInternal() override;
	void valueChanged(const juce::var&) override;

	void buttonClicked(juce::Button* b) override;
};