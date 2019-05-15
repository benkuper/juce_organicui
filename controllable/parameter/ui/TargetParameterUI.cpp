/*
  ==============================================================================

	TargetParameterUI.cpp
	Created: 2 Nov 2016 5:00:10pm
	Author:  bkupe

  ==============================================================================
*/

#include "TargetParameterUI.h"

TargetParameterUI::TargetParameterUI(TargetParameter * parameter, const String &_noTargetText) :
	ParameterUI(parameter),
	noTargetText(_noTargetText),
	targetParameter(parameter)
{
	setInterceptsMouseClicks(true, true);
	showEditWindowOnDoubleClick = false;

	targetBT.reset(AssetManager::getInstance()->getTargetBT());
	targetBT->setInterceptsMouseClicks(false, false);

	if (targetParameter->customCheckAssignOnNextChangeFunc != nullptr)
	{
		
		listeningToNextChange.reset(new BoolParameter("Learn", "When this parameter is on, any parameter that changes will be auto assigned to this target", false));
		listeningToNextChange->addAsyncParameterListener(this);
		listeningToNextChangeBT.reset(listeningToNextChange->createToggle());
		addAndMakeVisible(listeningToNextChangeBT.get());
	}


	addAndMakeVisible(targetBT.get());

	targetBT->addListener(this);
	setRepaintsOnMouseActivity(true);

	label.setFont(label.getFont().withHeight(12));
	updateLabel();
	label.setInterceptsMouseClicks(false, false);
	addAndMakeVisible(label);
}


TargetParameterUI::~TargetParameterUI()
{
	//in case we deleted with the listener still on
	if(parameter != nullptr && !parameter.wasObjectDeleted()) targetParameter->rootContainer->removeAsyncContainerListener(this);
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
 juce::Rectangle<int> r = getLocalBounds();
	if (listeningToNextChangeBT != nullptr)
	{
		listeningToNextChangeBT->setBounds(r.removeFromRight(50));
		r.removeFromRight(2);
	}

	targetBT->setBounds(r.removeFromLeft(r.getHeight()).reduced(2));
	r.removeFromLeft(2);
	label.setBounds(r.reduced(0, 2));
}

void TargetParameterUI::updateLabel()
{
	String newText;
	if (targetParameter->targetType == TargetParameter::TargetType::CONTROLLABLE)
	{
		if (targetParameter->target != nullptr)
		{
			if (targetParameter->customGetControllableLabelFunc != nullptr) newText = targetParameter->customGetControllableLabelFunc(targetParameter->target);
			else
			{
				if (targetParameter->showFullAddressInEditor) targetParameter->target->getControlAddress();
				else
				{
					if (targetParameter->showParentNameInEditor)
					{
						int curPLevel = 0;
						ControllableContainer * cc = targetParameter->target->parentContainer;
						while (curPLevel < targetParameter->defaultParentLabelLevel ||  (cc != nullptr && cc->skipLabelInTarget))
						{
							if(curPLevel > 0) cc = cc->parentContainer;
							curPLevel++;
						}
						if(cc != nullptr) newText = cc->niceName + ":";
					}

					newText += targetParameter->target->niceName;
				}
			}
		}
	} else //TargetType::CONTAINER
	{
		if (targetParameter->targetContainer != nullptr)
		{
			if (targetParameter->customGetContainerLabelFunc != nullptr) newText = targetParameter->customGetContainerLabelFunc(targetParameter->targetContainer);
			else
			{
				if (targetParameter->showFullAddressInEditor) targetParameter->target->getControlAddress();
				else
				{
					if (targetParameter->showParentNameInEditor)
					{
						int curPLevel = 0;
						ControllableContainer * cc = targetParameter->targetContainer;
						while (curPLevel < targetParameter->defaultParentLabelLevel || (cc != nullptr && cc->skipLabelInTarget))
						{
							cc = cc->parentContainer;
							curPLevel++;
						}
						if(cc != nullptr) newText = cc->niceName + ":";
					}

					newText += targetParameter->targetContainer->niceName;
				}
			}
		}
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
			ControllableChooserPopupMenu p(targetParameter->rootContainer, targetParameter->showParameters, targetParameter->showTriggers,0,targetParameter->maxDefaultSearchLevel);
			c = p.showAndGetControllable();
		}
		if (c != nullptr) targetParameter->setValueFromTarget(c);

	} else
	{
		ControllableContainer * cc = nullptr;
		if (targetParameter->customGetTargetContainerFunc != nullptr)
		{
			cc = targetParameter->customGetTargetContainerFunc();
		} else
		{
			ContainerChooserPopupMenu p(targetParameter->rootContainer, 0, targetParameter->maxDefaultSearchLevel, targetParameter->defaultContainerTypeCheckFunc);
			cc = p.showAndGetContainer();
		}
		if (cc != nullptr) targetParameter->setValueFromTarget(cc);
	}
}

void TargetParameterUI::mouseDownInternal(const MouseEvent &)
{
	showPopupAndGetTarget();
}

void TargetParameterUI::buttonClicked(Button * b)
{
	if (b == targetBT.get()) {} // move code here ?
}

void TargetParameterUI::valueChanged(const var &)
{
	if (listeningToNextChange != nullptr) listeningToNextChange->setValue(false);

	updateLabel();
	repaint();
}

void TargetParameterUI::newMessage(const Parameter::ParameterEvent & e)
{

	if (e.parameter == listeningToNextChange.get())
	{
		if (e.type == Parameter::ParameterEvent::VALUE_CHANGED)
		{
			{
				if (listeningToNextChange->boolValue())
				{
					targetParameter->rootContainer->addAsyncContainerListener(this);
				} else 
				{
					targetParameter->rootContainer->removeAsyncContainerListener(this);
				}
			}
		}
	} else
	{
		ParameterUI::newMessage(e);
	}
	
}

void TargetParameterUI::newMessage(const ContainerAsyncEvent & e)
{
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		Controllable * c = e.targetControllable;
		if (c == targetParameter->target) return;

		if (c->type == Controllable::TRIGGER)
		{
			if (!targetParameter->showTriggers) return;
		} else
		{
			if (!targetParameter->showParameters) return;
		}

		bool isControllableValid = targetParameter->customCheckAssignOnNextChangeFunc(c);
		if (isControllableValid) targetParameter->setValueFromTarget(c);
	}
	
}
