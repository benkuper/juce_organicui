#include "ParameterEditor.h"
/*
  ==============================================================================

	ParameterEditor.cpp
	Created: 7 Oct 2016 2:04:37pm
	Author:  bkupe

  ==============================================================================
*/


ParameterEditor::ParameterEditor(Parameter * _parameter, bool isRoot, int initHeight) :
	ControllableEditor(_parameter, isRoot, initHeight),
	parameter(_parameter)
{
	if (parameter->isOverriden) label.setFont(label.getFont().withStyle(Font::FontStyleFlags::bold));
	
	parameter->addParameterListener(this);
	parameter->addAsyncParameterListener(this);

	updateUI();
}

ParameterEditor::~ParameterEditor()
{
	if (parameter != nullptr && !parameter.wasObjectDeleted())
	{
		parameter->removeParameterListener(this);
		parameter->removeAsyncParameterListener(this);
	}
}

void ParameterEditor::resized()
{
	ControllableEditor::resized();
	if (parameter->controlMode == Parameter::MANUAL) return;

	juce::Rectangle<int> r = getLocalBounds().removeFromBottom(subContentHeight - 2);
	
	if (parameter->controlMode == Parameter::EXPRESSION && expressionText != nullptr) 
	{		
		expressionLabel->setBounds(r.removeFromLeft(60));
		expressionText->setBounds(r);
	}
}

void ParameterEditor::updateUI()
{
	subContentHeight = parameter->controlMode != Parameter::MANUAL ? 18 : 0;

	ui->setForceFeedbackOnly(parameter->isControllableFeedbackOnly || parameter->controlMode != Parameter::MANUAL);

	if (parameter->controlMode == Parameter::EXPRESSION)
	{
		if (expressionText == nullptr)
		{
			expressionLabel = new Label();
			expressionLabel->setEditable(false);
			expressionLabel->setText("=", dontSendNotification);
			addAndMakeVisible(expressionLabel);

			expressionText = new Label();
			expressionText->setText(parameter->controlExpression, dontSendNotification);
			expressionText->addListener(this);
			expressionText->setEditable(true);
			addAndMakeVisible(expressionText);

			expressionLabel->setJustificationType(Justification::topLeft);
			expressionLabel->setColour(expressionLabel->textColourId, TEXTNAME_COLOR);
			
			expressionText->setJustificationType(Justification::topLeft);
			expressionText->setColour(expressionText->textColourId, HIGHLIGHT_COLOR);
			expressionText->setColour(expressionText->backgroundColourId, BG_COLOR.brighter(.3f));
			expressionText->setColour(expressionText->backgroundWhenEditingColourId, Colours::black);
			expressionText->setColour(expressionText->textWhenEditingColourId, Colours::white);
			expressionText->setColour(CaretComponent::caretColourId, Colours::orange);

		}
	} else
	{
		
		if (expressionLabel != nullptr)
		{
			removeChildComponent(expressionLabel);
			expressionLabel = nullptr;

			removeChildComponent(expressionText);
			expressionText->removeListener(this);
			expressionText = nullptr;
		}
	}

	setSize(getWidth(), baseHeight + subContentHeight);
}

void ParameterEditor::newMessage(const Parameter::ParameterEvent & e)
{
	label.setFont(label.getFont().withStyle(parameter->isOverriden?Font::FontStyleFlags::bold:Font::FontStyleFlags::plain));
}

void ParameterEditor::parameterControlModeChanged(Parameter *)
{
	updateUI();
}

void ParameterEditor::labelTextChanged(Label * labelThatHasChanged)
{
	parameter->setControlExpression(expressionText->getText());
}
