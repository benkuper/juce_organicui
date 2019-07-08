#include "FloatStepperUI.h"
/*
  ==============================================================================

    FloatStepperUI.cpp
    Created: 28 Apr 2016 6:00:28pm
    Author:  bkupe

  ==============================================================================
*/

FloatStepperUI::FloatStepperUI(Parameter * _parameter) :
    ParameterUI(_parameter)
{
	showEditWindowOnDoubleClick = false;

	slider.reset(new BetterStepper(tooltip));
	slider->setEnabled(parameter->enabled);
	

    slider->setValue(parameter->floatValue());
    slider->addListener(this);
	slider->addMouseListener(this,true);
	slider->setColour(slider->textBoxBackgroundColourId,BG_COLOR.darker(.1f).withAlpha(.8f));
	slider->setColour(CaretComponent::caretColourId, Colours::orange);
	slider->setScrollWheelEnabled(false);
	slider->setColour(slider->textBoxTextColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));
	feedbackStateChanged();

	if (parameter->hasRange() && parameter->minimumValue != parameter->maximumValue) slider->setRange((int)parameter->minimumValue, (int)parameter->maximumValue, 0);

	addAndMakeVisible(slider.get());

    startTimerHz(20);
}

FloatStepperUI::~FloatStepperUI()
{
	stopTimer();
}

void FloatStepperUI::resized()
{
    slider->setBounds(getLocalBounds());
}


void FloatStepperUI::valueChanged(const var & value)
{
    if ((float)value == slider->getValue()) return;
    shouldUpdateStepper = true;
}

void FloatStepperUI::sliderValueChanged(Slider * _slider)
{
	if (parameter.wasObjectDeleted()) return;
	parameter->setValue(_slider->getValue());
}

void FloatStepperUI::controllableStateChanged()
{
	ParameterUI::controllableStateChanged();
	slider->setEnabled(parameter->enabled);
	slider->setColour(slider->textBoxTextColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));
}

void FloatStepperUI::rangeChanged(Parameter *){
	slider->setRange((int)parameter->minimumValue, (int)parameter->maximumValue, 0);

}

void FloatStepperUI::feedbackStateChanged()
{
	slider->setTextBoxIsEditable(isInteractable());
	slider->setIncDecButtonsMode(isInteractable()?Slider::IncDecButtonMode::incDecButtonsDraggable_Vertical:Slider::IncDecButtonMode::incDecButtonsNotDraggable);
}

void FloatStepperUI::timerCallback()
{
    if (!shouldUpdateStepper) return;
    shouldUpdateStepper = false;
	
	if (parameter.wasObjectDeleted()) return;

    slider->setValue(parameter->floatValue(), NotificationType::dontSendNotification);
}

