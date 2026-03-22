/*
  ==============================================================================

	AutomationKeyUI.cpp
	Created: 21 Mar 2020 4:06:36pm
	Author:  bkupe

  ==============================================================================
*/

AutomationKeyUI::AutomationKeyUI(AutomationKey* key) :
	BaseItemMinimalUI(key),
	handle(key),
	easingUI(nullptr)
{
	bringToFrontOnSelect = false;

	autoDrawContourWhenSelected = false;
	dragAndDropEnabled = false;

	addAndMakeVisible(&handle);

	updateEasingUI();
}

AutomationKeyUI::~AutomationKeyUI()
{
	if (!inspectable.wasObjectDeleted()) if (item->easing != nullptr) item->easing->removeInspectableListener(this);
}

void AutomationKeyUI::paint(Graphics& g) 
{
	//g.fillAll(Colours::purple.withAlpha(.1f));
}

void AutomationKeyUI::resized()
{
	Point<int> hp = getUIPosForValuePos(item->getPosAndValue());
	const int handleSize = 14;
	handle.setBounds(juce::Rectangle<int>(hp.x - handleSize / 2, hp.y - handleSize / 2, handleSize, handleSize));
	if (easingUI != nullptr) easingUI->setBounds(getLocalBounds()); 
}


void AutomationKeyUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	if (inspectable.wasObjectDeleted()) return;
	if (easingUI != nullptr) easingUI->setShowEasingHandles(showFirst && item->nextKey != nullptr, showLast && item->nextKey != nullptr);
}

void AutomationKeyUI::updateEasingUI()
{
	if (easingUI != nullptr)
	{
		removeChildComponent(this);
	}

	if (item->easing == nullptr)
	{
		easingUI.reset();
		return;
	}

	easingUI.reset(item->easing->createUI());

	if (easingUI != nullptr)
	{
		addAndMakeVisible(easingUI.get(), 0);
		easingUI->setBounds(getLocalBounds());
		easingUI->setValueBounds(valueBounds);
	}

}

void AutomationKeyUI::mouseDown(const MouseEvent& e)
{
	if (e.eventComponent == easingUI.get())
	{
		if (e.mods.isRightButtonDown())
		{
			PopupMenu ep;
			juce::StringArray keys = item->easingType->getAllKeys();
			int kid = 1;
			for (auto& i : keys)
			{
				ep.addItem(kid, i, true, i == item->easingType->getValueKey());
				kid++;
			}

			ep.showMenuAsync(PopupMenu::Options(), [this, keys](int result)
				{
					if (result >= 1 && result <= keys.size())
					{
						this->item->easingType->setUndoableValue(item->easingType->value, keys[result - 1]);
						this->item->easing->selectThis(); //reselect after changing easing
					}
				}
			);
		}
		else if (e.mods.isCommandDown())
		{
			item->easingType->setNext(true, true);
			item->easing->selectThis(); //reselect after changing easing
		}
	}
	else
	{
		item->setMovePositionReference(true);
		BaseItemMinimalUI::mouseDown(e);
	}
}

void AutomationKeyUI::mouseDoubleClick(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDoubleClick(e);
	if (e.eventComponent == this || e.eventComponent == &handle)
	{
		std::unique_ptr<Component> editComponent(new KeyEditCalloutComponent(item));
		CallOutBox* box = &CallOutBox::launchAsynchronously(std::move(editComponent), localAreaToGlobal(getLocalBounds()), nullptr);
		box->setArrowSize(8);
	}
}

void AutomationKeyUI::mouseUp(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseUp(e);
	item->addMoveToUndoManager(true);
}

bool AutomationKeyUI::hitTest(int x, int y)
{
	if (handle.getBounds().contains(x, y)) return true;
	if (easingUI != nullptr && easingUI->hitTest(x, y)) return true;
	return false;
}

void AutomationKeyUI::setValueBounds(const juce::Rectangle<float> _valueBounds)
{
	valueBounds = _valueBounds;
	if (easingUI != nullptr) easingUI->setValueBounds(valueBounds);
	resized();
}


Point<int> AutomationKeyUI::getUIPosForValuePos(const Point<float>& valuePos) const
{
	return getLocalBounds().getRelativePoint((valuePos.x - valueBounds.getX()) / valueBounds.getWidth(), 1 - ((valuePos.y - valueBounds.getY()) / valueBounds.getHeight()));
}


void AutomationKeyUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == item->easingType) updateEasingUI();
	else if (CubicEasing* ce = dynamic_cast<CubicEasing*>(c->parentContainer.get()))
	{
		if (CubicEasingUI* eui = dynamic_cast<CubicEasingUI*>(easingUI.get()))
		{
			bool isFirst = c == ce->anchor1;
			keyUIListeners.call(&KeyUIListener::keyEasingHandleMoved, this, eui->syncHandles, isFirst);
		}
	}
}

AutomationKeyHandle::AutomationKeyHandle(AutomationKey* key) :
	InspectableContentComponent(key),
	key(key)
{
	autoDrawContourWhenSelected = false;
	setRepaintsOnMouseActivity(true);

}

AutomationKeyHandle::~AutomationKeyHandle()
{
}

void AutomationKeyHandle::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	Colour bc = NORMAL_COLOR;
	Colour c = key->isSelected ? HIGHLIGHT_COLOR : key->isPreselected? PRESELECT_COLOR : bc;
	if (isMouseOverOrDragging()) c = c.brighter(.2f);

	juce::Rectangle<float> r = getLocalBounds().reduced(3).toFloat();
	g.setColour(c);
	g.fillEllipse(r);
	g.setColour(c.darker());
	g.drawEllipse(r, 1);
}

void AutomationKeyHandle::inspectableSelectionChanged(Inspectable* i)
{
	if(MessageManager::getInstance()->isThisTheMessageThread()) repaint();
}



//
AutomationKeyUI::KeyEditCalloutComponent::KeyEditCalloutComponent(AutomationKey * k) :
	k(k)
{
	positionEditor.reset((ParameterEditor *)k->position->getEditor(false));
	valueEditor.reset((ParameterEditor *)k->value->getEditor(false));
	addAndMakeVisible(positionEditor.get());
	addAndMakeVisible(valueEditor.get());
	setSize(160, 50);
}

AutomationKeyUI::KeyEditCalloutComponent::~KeyEditCalloutComponent()
{
}

void AutomationKeyUI::KeyEditCalloutComponent::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	positionEditor->setBounds(r.removeFromTop(25).reduced(2));
	valueEditor->setBounds(r.reduced(2));
}

