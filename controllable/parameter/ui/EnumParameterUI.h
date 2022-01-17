/*
  ==============================================================================

    EnumParameterUI.h
    Created: 29 Sep 2016 5:35:12pm
    Author:  bkupe

  ==============================================================================
*/


#pragma once

class EnumOptionManager :
	public Component,
	public Label::Listener
{
public:
	EnumOptionManager(EnumParameter* ep);
	~EnumOptionManager();

	EnumParameter* ep;
	Viewport viewport;
	Component container;

	class EnumOptionUI :
		public Component
	{
	public:
		EnumOptionUI(EnumParameter* ui, int index);

		EnumParameter* ep;
		int index;

		Label keyLabel;
		Label valueLabel;

		void resized() override;
	};

	OwnedArray<EnumOptionUI> optionsUI;

	void paint(Graphics& g) override;
	void resized() override;
	void labelTextChanged(Label* l) override;
};

class EnumParameterUIBase :
	public ParameterUI,
	public EnumParameter::AsyncListener
{
public:
	EnumParameterUIBase(Array<EnumParameter*> parameters);
	virtual ~EnumParameterUIBase();

	Array<EnumParameter*> eps;
	EnumParameter* ep;

	virtual void updateFromParameter() {}

	virtual void addPopupMenuItemsInternal(PopupMenu* p) override;
	virtual void handleMenuSelectedID(int result) override;

	void newMessage(const EnumParameter::EnumParameterEvent& e) override;
};

class EnumParameterUI :
	public EnumParameterUIBase,
	public ComboBox::Listener
{
public:
    EnumParameterUI(Array<EnumParameter *> parameters);
    virtual ~EnumParameterUI();

	ComboBox cb;

	void updateFromParameter() override;
	String getSelectedKey();

	var prevValue;

	void resized() override;

	// Inherited via Listener
	virtual void comboBoxChanged(ComboBox *) override;


private:
	HashMap<int, String> idKeyMap;
	HashMap<String,int> keyIdMap;

protected:
	void updateUIParamsInternal() override;
    void valueChanged(const var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnumParameterUI)
};



class EnumParameterButtonBarUI :
	public EnumParameterUIBase,
	public TextButton::Listener
{
public:
	EnumParameterButtonBarUI(Array<EnumParameter*> parameters);
	~EnumParameterButtonBarUI();

	OwnedArray<TextButton> buttons;
	bool isVertical;

	void updateFromParameter() override;
	void resized() override;

protected:
	void updateUIParamsInternal() override;
	void valueChanged(const var&) override;

	void buttonClicked(Button* b) override;
};