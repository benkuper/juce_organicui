/*
  ==============================================================================

    EnumParameterUI.cpp
    Created: 29 Sep 2016 5:35:12pm
    Author:  bkupe

  ==============================================================================
*/


EnumParameterUI::EnumParameterUI(Parameter * parameter) :
	ParameterUI(parameter),
	ep((EnumParameter *)parameter)
{
	cb.addListener(this);
	cb.setTextWhenNoChoicesAvailable(ep->niceName);
	cb.setTextWhenNothingSelected(ep->niceName);
	cb.setTooltip(ep->description);
	addAndMakeVisible(cb);

	ep->addEnumParameterListener(this);

	prevValue = parameter->value;

	updateComboBox();
}

EnumParameterUI::~EnumParameterUI()
{
	ep->removeEnumParameterListener(this);

	cb.removeListener(this);
}

void EnumParameterUI::updateComboBox()
{	
	cb.clear(dontSendNotification);
	HashMap<String, var>::Iterator i(ep->enumValues);
	idKeyMap.clear();
	int id = 1;

	while (i.next())
	{
		cb.addItem(i.getKey(), id);
		idKeyMap.set(id, i.getKey());
		keyIdMap.set(i.getKey(), id);
		id++;
	}

	cb.setSelectedId(keyIdMap[ep->stringValue()], dontSendNotification);
	cb.setEnabled(isInteractable());
	cb.setInterceptsMouseClicks(isInteractable(), isInteractable());
}

String EnumParameterUI::getSelectedKey()
{
	return idKeyMap[cb.getSelectedId()];
}

void EnumParameterUI::resized()
{
	cb.setBounds(getLocalBounds());
}

void EnumParameterUI::enumOptionAdded(EnumParameter *, const String &)
{
	updateComboBox();
}

void EnumParameterUI::enumOptionRemoved(EnumParameter *, const String &)
{
	updateComboBox();
}

void EnumParameterUI::feedbackStateChanged()
{
	updateComboBox();
}

void EnumParameterUI::valueChanged(const var & value)
{
	cb.setSelectedId(keyIdMap[value], dontSendNotification);
	prevValue = parameter->value;
}

void EnumParameterUI::comboBoxChanged(ComboBox *)
{
	ep->setUndoableValue(prevValue, getSelectedKey());
	
};