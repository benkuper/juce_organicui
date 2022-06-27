/*
  ==============================================================================

    BetterIntStepper.cpp
    Created: 28 Sep 2016 3:14:10pm
    Author:  bkupe

  ==============================================================================
*/

BetterStepper::BetterStepper(const String& tooltip) :
	Slider(SliderStyle::IncDecButtons, TextEntryBoxPosition::TextBoxLeft)
{
	bool showTip = GlobalSettings::getInstanceWithoutCreating() == nullptr && GlobalSettings::getInstance()->enableTooltips->boolValue();
	if(showTip) setTooltip(tooltip);
	setIncDecButtonsMode(IncDecButtonMode::incDecButtonsDraggable_Horizontal);

	valueFromTextFunction = &ParameterUI::textToValue;
}

BetterStepper::~BetterStepper()
{
}

void BetterStepper::setTooltip(const String & tooltip)
{
	Slider::setTooltip(tooltip);
}


