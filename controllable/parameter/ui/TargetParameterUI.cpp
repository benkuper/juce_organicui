/*
  ==============================================================================

	TargetParameterUI.cpp
	Created: 2 Nov 2016 5:00:10pm
	Author:  bkupe

  ==============================================================================
*/


TargetParameterUI::TargetParameterUI(TargetParameter * parameter, const String &_noTargetText) :
	ParameterUI(parameter),
	noTargetText(_noTargetText),
	targetParameter(parameter)
{
	setInterceptsMouseClicks(true, true);

	targetBT = AssetManager::getInstance()->getTargetBT();
	targetBT->setInterceptsMouseClicks(false, false);

	addAndMakeVisible(targetBT);

	targetBT->addListener(this);
	setRepaintsOnMouseActivity(true);

	label.setFont(label.getFont().withHeight(12));
	updateLabel();
	label.setInterceptsMouseClicks(false, false);
	addAndMakeVisible(label);

}

TargetParameterUI::~TargetParameterUI()
{
}

void TargetParameterUI::paint(Graphics & g)
{

	Colour c = targetParameter->target != nullptr || targetParameter->targetContainer != nullptr ? GREEN_COLOR : NORMAL_COLOR;
	if (isMouseOver()) c = c.brighter();

	g.setGradientFill(ColourGradient(c.brighter(), (float)getLocalBounds().getCentreX(), (float)getLocalBounds().getCentreY(), c.darker(), 2.f, 2.f, true));
	g.fillRoundedRectangle(targetBT->getBounds().expanded(2).toFloat(), 2);

}

void TargetParameterUI::resized()
{
	Rectangle<int> r = getLocalBounds();
	targetBT->setBounds(r.removeFromLeft(r.getHeight()).reduced(2));
	r.removeFromLeft(2);
	label.setBounds(r.reduced(0, 2));
}

void TargetParameterUI::updateLabel()
{
	String newText;
	if (targetParameter->targetType == TargetParameter::TargetType::CONTROLLABLE)
	{
		if (targetParameter->target != nullptr) newText = targetParameter->showFullAddressInEditor ? targetParameter->target->getControlAddress() : targetParameter->target->parentContainer->niceName + ":" + targetParameter->target->niceName;
	} else //TargetType::CONTAINER
	{
		if (targetParameter->targetContainer != nullptr) newText = targetParameter->targetContainer->getControlAddress();
	}

	if (newText.isEmpty())
	{
		if (targetParameter->ghostValue.isNotEmpty()) newText = "### " + targetParameter->ghostValue;
		else newText = noTargetText;
	}

	label.setText(newText, dontSendNotification);
}

void TargetParameterUI::showPopupAndGetTarget()
{
	if (!parameter->enabled) return;

	if (targetParameter->targetType == TargetParameter::TargetType::CONTROLLABLE)
	{
		Controllable * c = nullptr;

		if (targetParameter->customGetTargetFunc != nullptr)
		{
			c = targetParameter->customGetTargetFunc(targetParameter->showTriggers,targetParameter->showParameters);
		} else
		{
			ControllableChooserPopupMenu p(targetParameter->rootContainer, targetParameter->showParameters, targetParameter->showTriggers);
			c = p.showAndGetControllable();
		}
		
		if (c != nullptr) targetParameter->setValueFromTarget(c);
	} else
	{
		//No handle for container for now
	}
}

void TargetParameterUI::mouseDownInternal(const MouseEvent &)
{
	if (!targetParameter->isEditable || forceFeedbackOnly) return;

	showPopupAndGetTarget();
}

void TargetParameterUI::buttonClicked(Button * b)
{
	if (b == targetBT) {} // move code here ?
}

void TargetParameterUI::valueChanged(const var &)
{
	updateLabel();
	repaint();
}
