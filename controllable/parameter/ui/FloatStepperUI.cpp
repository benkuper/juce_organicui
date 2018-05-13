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

	slider = new BetterStepper(tooltip);
	slider->setEnabled(parameter->enabled);
	
    slider->setRange((int)parameter->minimumValue, (int)parameter->maximumValue,1);

    slider->setValue(parameter->floatValue());
    slider->addListener(this);
	slider->addMouseListener(this,true);
	slider->setColour(slider->textBoxBackgroundColourId,BG_COLOR.darker(.1f).withAlpha(.8f));
	slider->setColour(CaretComponent::caretColourId, Colours::orange);
	slider->setScrollWheelEnabled(false);

	bool active = parameter->isEditable && !forceFeedbackOnly;
	slider->setTextBoxIsEditable(active);
	slider->setIncDecButtonsMode(active ? Slider::IncDecButtonMode::incDecButtonsDraggable_Vertical : Slider::IncDecButtonMode::incDecButtonsNotDraggable);
	
	addAndMakeVisible(slider);
}

FloatStepperUI::~FloatStepperUI()
{

}

void FloatStepperUI::resized()
{
	slider->setTextBoxIsEditable(parameter->isEditable && !forceFeedbackOnly);
	slider->setIncDecButtonsMode(parameter->isEditable && !forceFeedbackOnly ? Slider::IncDecButtonMode::incDecButtonsDraggable_Vertical : Slider::IncDecButtonMode::incDecButtonsNotDraggable);
    slider->setBounds(getLocalBounds());
}


void FloatStepperUI::valueChanged(const var & value)
{
    if ((float)value == slider->getValue()) return;

    slider->setValue(value,NotificationType::dontSendNotification);
}

void FloatStepperUI::sliderValueChanged(Slider * _slider)
{
	parameter->setValue(_slider->getValue());
}
void FloatStepperUI::controllableStateChanged()
{
	ParameterUI::controllableStateChanged();
	slider->setEnabled(parameter->enabled);
}

void FloatStepperUI::rangeChanged(Parameter *){
	slider->setRange((int)parameter->minimumValue - 1, (int)parameter->maximumValue + 1, 1);

}

void FloatStepperUI::setForceFeedbackOnlyInternal()
{
	bool active = parameter->isEditable && !forceFeedbackOnly;
	slider->setTextBoxIsEditable(active);
	slider->setIncDecButtonsMode(active?Slider::IncDecButtonMode::incDecButtonsDraggable_Vertical:Slider::IncDecButtonMode::incDecButtonsNotDraggable);
}
