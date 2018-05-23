ParameterEditor::ParameterEditor(Parameter * _parameter, bool isRoot, int initHeight) :
	ControllableEditor(_parameter, isRoot, initHeight),
	parameter(_parameter)
{
	if (parameter->isOverriden && parameter->isEditable && !parameter->isControllableFeedbackOnly) label.setFont(label.getFont().withStyle(Font::FontStyleFlags::bold));
	
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
	
	switch (parameter->controlMode)
	{
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

	ui->setForceFeedbackOnly(parameter->isControllableFeedbackOnly || parameter->controlMode != Parameter::MANUAL);

	//cleanup
	if (expressionLabel != nullptr)
	{
		removeChildComponent(expressionLabel);
		expressionLabel = nullptr;

		removeChildComponent(expressionText);
		expressionText->removeListener(this);
		expressionText = nullptr;
	}

	if (automationUI != nullptr)
	{
		removeChildComponent(automationUI);
		automationUI = nullptr;
	}

	if (referenceUI != nullptr)
	{
		removeChildComponent(referenceUI);
		referenceUI = nullptr;
	}

	switch (parameter->controlMode)
	{
	case Parameter::EXPRESSION:
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

			subContentHeight = 18;
		}
	}
	break;

	case Parameter::REFERENCE:
	{
		referenceUI = parameter->referenceTarget->createTargetUI();
		addAndMakeVisible(referenceUI);
		subContentHeight = 18;
	}
	break;

	case Parameter::AUTOMATION:
	{
		automationUI = new PlayableParameterAutomationEditor(parameter->automation);
		addAndMakeVisible(automationUI);
		subContentHeight = 100;
	}
	break;
	}

	setSize(getWidth(), baseHeight + subContentHeight);
}

void ParameterEditor::newMessage(const Parameter::ParameterEvent & e)
{
	if (!parameter->isEditable || parameter->isControllableFeedbackOnly) return;

	label.setFont(label.getFont().withStyle(parameter->isOverriden ? Font::FontStyleFlags::bold : Font::FontStyleFlags::plain));

	if (e.type == Parameter::ParameterEvent::BOUNDS_CHANGED)
	{
		/*
		removeChildComponent(ui);

		ui = controllable->createDefaultUI();
		ui->showLabel = false;
		ui->setOpaqueBackground(true);
		addAndMakeVisible(ui);
		updateUI();
		*/
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
