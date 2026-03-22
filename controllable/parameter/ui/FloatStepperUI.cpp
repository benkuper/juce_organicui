/*
  ==============================================================================

	FloatStepperUI.cpp
	Created: 28 Apr 2016 6:00:28pm
	Author:  bkupe

  ==============================================================================
*/

#include  "JuceHeader.h"

FloatStepperUI::FloatStepperUI(Array<Parameter*> parameters) :
	ParameterUI(parameters, ORGANICUI_DEFAULT_TIMER),
	valueAtDragStart(parameters[0]->floatValue())
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
			slider->setRange(INT32_MIN / 2, INT32_MAX / 2, 1);
			slider->setMouseDragSensitivity(INT32_MAX);
		}
	}

	slider->setNumDecimalPlacesToDisplay(0);
	slider->setValue(parameter->floatValue(), dontSendNotification);

	slider->setColour(slider->textBoxBackgroundColourId, useCustomFGColor ? customFGColor : BG_COLOR.darker(.1f).withAlpha(.8f));
	slider->setColour(CaretComponent::caretColourId, Colours::orange);
	slider->setScrollWheelEnabled(false);
	slider->setColour(slider->textBoxTextColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));


	feedbackStateChanged();
	addAndMakeVisible(slider.get());


	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);

	slider->addListener(this);
	slider->addMouseListener(this, true);
	slider->getTextbox()->onEditorHide = [this]() {sliderValueChanged(slider.get());  };
}

FloatStepperUI::~FloatStepperUI()
{
}

void FloatStepperUI::paint(Graphics& g)
{
	ParameterUI::paint(g);

	if (parameter == nullptr || parameter.wasObjectDeleted()) return;

	if (useCustomBGColor) g.fillAll(customBGColor);

	if (showLabel)
	{
		juce::Rectangle<int> r = getLocalBounds();
		g.setFont(customTextSize > 0 ? customTextSize : Font());
		r = r.removeFromLeft(jmin(g.getCurrentFont().getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 
		 60));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, r, Justification::centred, 1);
	}
}

void FloatStepperUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	if (parameter == nullptr || parameter.wasObjectDeleted()) return;

	if (showLabel)
	{
		Font font(customTextSize > 0 ? customTextSize : jlimit(12, 40, jmin(r.getHeight() - 4, r.getWidth()) - 16));
		r.removeFromLeft(jmin(font.getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName), r.getWidth() - 60));
		slider->setBounds(r.removeFromRight(jmin(r.getWidth(), 60)));
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
	slider->setColour(slider->textBoxBackgroundColourId, useCustomFGColor ? customFGColor : BG_COLOR.darker(.1f).withAlpha(.8f));
}


void FloatStepperUI::valueChanged(const var& value)
{
	if ((float)value == slider->getValue()) return;
	shouldRepaint = true;
}

void FloatStepperUI::sliderValueChanged(Slider* _slider)
{
	if (parameter.wasObjectDeleted()) return;
	parameter->setValue(slider->getValue());
}

void FloatStepperUI::sliderDragStarted(Slider* _slider)
{
	if (parameter.wasObjectDeleted()) return;
	valueAtDragStart = parameter->floatValue();
}

void FloatStepperUI::sliderDragEnded(Slider* _slider)
{
	if (parameter.wasObjectDeleted()) return;
	parameter->setUndoableValue(valueAtDragStart, parameter->floatValue());
}

void FloatStepperUI::rangeChanged(Parameter*) {
	slider->setRange((int)parameter->minimumValue, (int)parameter->maximumValue, 1);
}

void FloatStepperUI::handlePaintTimerInternal()
{
	slider->setValue(parameter->floatValue(), NotificationType::dontSendNotification);
}

