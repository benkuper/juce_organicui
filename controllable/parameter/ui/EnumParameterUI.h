/*
  ==============================================================================

    EnumParameterUI.h
    Created: 29 Sep 2016 5:35:12pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef ENUMPARAMETERUI_H_INCLUDED
#define ENUMPARAMETERUI_H_INCLUDED


class EnumParameterUI : public ParameterUI, public EnumParameter::Listener, public ComboBox::Listener
{
public:
    EnumParameterUI(Parameter * parameter = nullptr);
    virtual ~EnumParameterUI();

	EnumParameter * ep;

	ComboBox cb;

	void updateComboBox();
	String getSelectedKey();

	var prevValue;

	void resized() override;
	
	void enumOptionAdded(EnumParameter *, const String &key) override;
	void enumOptionRemoved(EnumParameter *, const String &key) override;

	// Inherited via Listener
	virtual void comboBoxChanged(ComboBox *) override;
	
private:
	HashMap<int, String> idKeyMap;
	HashMap<String,int> keyIdMap;

protected:
	void setForceFeedbackOnlyInternal() override;
    void valueChanged(const var &) override ;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnumParameterUI)

		
};

#endif  // ENUMPARAMETERUI_H_INCLUDED
