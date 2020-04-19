#include "ParameterEditor.h"
ParameterEditor::ParameterEditor(Parameter* _parameter, bool isRoot) :
	ControllableEditor(_parameter, isRoot),
	parameter(_parameter),
	currentUIHasRange(_parameter->hasRange())
{
	if (parameter->isOverriden && ui->isInteractable()) label.setFont(label.getFont().withStyle(Font::FontStyleFlags::bold));
	
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

	if (parameter.wasObjectDeleted()) return;

	if (parameter->controlMode == Parameter::MANUAL) return;

	juce::Rectangle<int> r = getLocalBounds().removeFromBottom(subContentHeight - 2);
	
	switch (parameter->controlMode)
	{
        case Parameter::MANUAL:
            break;
            
	case Parameter::EXPRESSION:
		if (expressionText != nullptr)
		{
			expressionLabel->setBounds(r.removeFromLeft(60));
			expressionText->setBounds(r);
		}
		break;

	case Parameter::REFERENCE:
		if (referenceUI != nullptr) referenceUI->setBounds(r);
		break;

	case Parameter::AUTOMATION:
		if (automationUI != nullptr) automationUI->setBounds(r);
		break;
	}
	
}

void ParameterEditor::updateUI()
{
	subContentHeight = 0;

	//cleanup
	if (expressionLabel != nullptr)
	{
		removeChildComponent(expressionLabel.get());
		expressionLabel = nullptr;

		removeChildComponent(expressionText.get());
		expressionText->removeListener(this);
		expressionText = nullptr;
	}

	if (automationUI != nullptr)
	{
		removeChildComponent(automationUI.get());
		automationUI = nullptr;
	}

	if (referenceUI != nullptr)
	{
		removeChildComponent(referenceUI.get());
		referenceUI = nullptr;
	}

	switch (parameter->controlMode)
	{
        case Parameter::MANUAL:
            break;
            
	case Parameter::EXPRESSION:
	{
		if (expressionText == nullptr)
		{
			expressionLabel.reset(new Label());
			expressionLabel->setEditable(false);
			expressionLabel->setText("=", dontSendNotification);
			addAndMakeVisible(expressionLabel.get());

			expressionText.reset(new Label());
			expressionText->setText(parameter->controlExpression, dontSendNotification);
			expressionText->addListener(this);
			expressionText->setEditable(true);
			addAndMakeVisible(expressionText.get());

			expressionLabel->setJustificationType(Justification::topLeft);
			expressionLabel->setColour(expressionLabel->textColourId, TEXTNAME_COLOR);

			expressionText->setJustificationType(Justification::topLeft);
			expressionText->setColour(expressionText->textColourId, HIGHLIGHT_COLOR);
			expressionText->setColour(expressionText->backgroundColourId, BG_COLOR.brighter(.3f));
			expressionText->setColour(expressionText->backgroundWhenEditingColourId, Colours::black);
			expressionText->setColour(expressionText->textWhenEditingColourId, Colours::white);
			expressionText->setColour(CaretComponent::caretColourId, Colours::orange);

			subContentHeight = 18;
		}
	}
	break;

	case Parameter::REFERENCE:
	{
		referenceUI.reset(parameter->referenceTarget->createTargetUI());
		addAndMakeVisible(referenceUI.get());
		subContentHeight = 18;
	}
	break;

	case Parameter::AUTOMATION:
	{
		automationUI.reset(new ParameterAutomationEditor(parameter->automation.get()));
		addAndMakeVisible(automationUI.get());
		subContentHeight = 100;
	}
	break;
	}

	int targetHeight = baseHeight + subContentHeight;
	if (targetHeight == getHeight()) resized();
	else setSize(getWidth(), targetHeight);
}


void ParameterEditor::newMessage(const Parameter::ParameterEvent & e)
{
	if (ui->isInteractable()) label.setFont(label.getFont().withStyle(parameter->isOverriden ? Font::FontStyleFlags::bold : Font::FontStyleFlags::plain));
	if (e.type == Parameter::ParameterEvent::BOUNDS_CHANGED)
	{
		if (currentUIHasRange != parameter->hasRange() && parameter->rebuildUIOnRangeChange)
		{
			buildControllableUI(true);
			currentUIHasRange = parameter->hasRange();
		}
	}
}

void ParameterEditor::parameterControlModeChanged(Parameter *)
{
	updateUI();
}

void ParameterEditor::labelTextChanged(Label * labelThatHasChanged)
{
	parameter->setControlExpression(expressionText->getText());
}
