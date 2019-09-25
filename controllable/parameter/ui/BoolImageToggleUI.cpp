/*
  ==============================================================================

	BoolImageToggleUI.cpp
	Created: 4 Jan 2017 1:33:12pm
	Author:  Ben

  ==============================================================================
*/

BoolImageToggleUI::BoolImageToggleUI(ImageButton * i, BoolParameter *p) :
	ParameterUI(p),
	boolParam(p),
	bt(i)
{
	showEditWindowOnDoubleClick = false;

	addAndMakeVisible(bt.get());
	if (!boolParam->isControllableFeedbackOnly) bt->addListener(this);
	else bt->setInterceptsMouseClicks(false, false);
	setInterceptsMouseClicks(true, true); 
	bt->setOpaque(false);
	bt->setToggleState(boolParam->boolValue(), dontSendNotification);
	bt->setTooltip(tooltip);
}

BoolImageToggleUI::~BoolImageToggleUI()
{
}

void BoolImageToggleUI::setTooltip(const String & value)
{
	ParameterUI::setTooltip(value);
	bt->setTooltip(value);
}

void BoolImageToggleUI::resized()
{
	bt->setBounds(getLocalBounds());
}

void BoolImageToggleUI::valueChanged(const var &)
{
	bt->setToggleState(boolParam->boolValue(), dontSendNotification);
}

void BoolImageToggleUI::buttonClicked(Button *)
{
	boolParam->setUndoableValue(boolParam->boolValue(),!boolParam->boolValue());
}
