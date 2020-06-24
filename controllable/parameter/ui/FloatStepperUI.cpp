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

	if ((int)parameter->minimumValue != (int)parameter->maximumValue)
	{
		if (parameter->hasRange())
		{
			if (parameter->type == Controllable::INT) slider->setRange((int)parameter->minimumValue, (int)parameter->maximumValue, 1);
			else slider->setRange((float)parameter->minimumValue, (float)parameter->maximumValue, 1);
			int val = juce::jmin<int>(abs((int)parameter->maximumValue - (int)parameter->minimumValue), INT32_MAX);
			slider->setMouseDragSensitivity(val);
		}
		else
		{
			//workaround to keep good UI feeling
			slider->setRange(INT32_MIN/2, INT32_MAX/2, 1);
			slider->setMouseDragSensitivity(INT32_MAX);
		}
		
	}
    slider->setValue(parameter->floatValue());
    slider->addListener(this);
	slider->addMouseListener(this,true);
	
	slider->setColour(slider->textBoxBackgroundColourId,BG_COLOR.darker(.1f).withAlpha(.8f));
	slider->setColour(CaretComponent::caretColourId, Colours::orange);
	slider->setScrollWheelEnabled(false);
	slider->setColour(slider->textBoxTextColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));


	feedbackStateChanged();
	addAndMakeVisible(slider.get());

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);
    startTimerHz(20);
}

FloatStepperUI::~FloatStepperUI()
{
	stopTimer();
}

void FloatStepperUI::paint(Graphics& g)
{
	ParameterUI::paint(g);

	if (showLabel)
	{
		Rectangle<int> r = getLocalBounds();
		g.setFont(jlimit(12, 40, jmin(r.getHeight()-4, r.getWidth()) - 16)); 
		r = r.removeFromLeft(jmin(g.getCurrentFont().getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 60));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, r, Justification::centred, 1);
	}
}

void FloatStepperUI::resized()
{
	Rectangle<int> r = getLocalBounds();
	
	if (parameter == nullptr || parameter.wasObjectDeleted()) return;

	if (showLabel)
	{
		Font font(jlimit(12, 40, jmin(r.getHeight() - 4, r.getWidth()) - 16));

		r.removeFromLeft(jmin(font.getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName), r.getWidth()-60));
		slider->setBounds(r.removeFromRight(jmin(r.getWidth(),120)));
	}
	else
	{
		slider->setBounds(r);
	}
}

void FloatStepperUI::updateUIParamsInternal()
{
	slider->setEnabled(parameter->enabled);
	slider->setTextBoxIsEditable(isInteractable());
	slider->setIncDecButtonsMode((isInteractable()) ? Slider::IncDecButtonMode::incDecButtonsDraggable_AutoDirection : Slider::IncDecButtonMode::incDecButtonsNotDraggable);
	slider->setColour(slider->textBoxTextColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));
}


void FloatStepperUI::valueChanged(const var & value)
{
    if ((float)value == slider->getValue()) return;
    shouldUpdateStepper = true;
}

void FloatStepperUI::sliderValueChanged(Slider * _slider)
{
	if (parameter.wasObjectDeleted()) return;
	parameter->setValue(slider->getValue());
}

void FloatStepperUI::rangeChanged(Parameter *){
	slider->setRange((int)parameter->minimumValue, (int)parameter->maximumValue, 1);
}

void FloatStepperUI::timerCallback()
{
    if (!shouldUpdateStepper) return;
    shouldUpdateStepper = false;
	
	if (parameter.wasObjectDeleted()) return;

    slider->setValue(parameter->floatValue(), NotificationType::dontSendNotification);
}

