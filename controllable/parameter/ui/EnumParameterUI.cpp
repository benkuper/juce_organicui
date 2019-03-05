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

	prevValue = ep->getValueKey();

	updateComboBox();
}

EnumParameterUI::~EnumParameterUI()
{
	if(!parameter.wasObjectDeleted()) ep->removeEnumParameterListener(this);
	cb.removeListener(this);
}

void EnumParameterUI::updateComboBox()
{	
	cb.clear(dontSendNotification);
	idKeyMap.clear();

	int id = 1;
	for(auto &ev : ep->enumValues)
	{
		cb.addItem(ev->key, id);
		idKeyMap.set(id, ev->key);
		keyIdMap.set(ev->key, id);
		id++;
	}

	cb.setSelectedId(keyIdMap[ep->getValueKey()], dontSendNotification);
	cb.setEnabled(isInteractable());
	cb.setInterceptsMouseClicks(isInteractable(), isInteractable());

	updateTooltip();
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
	cb.setSelectedId(keyIdMap[ep->getValueKey()], dontSendNotification);
	prevValue = ep->getValueKey();
}

void EnumParameterUI::comboBoxChanged(ComboBox *)
{
	ep->setUndoableValue(prevValue, getSelectedKey());
	
};