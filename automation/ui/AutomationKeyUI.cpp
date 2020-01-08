/*
  ==============================================================================

	AutomationKeyTimelineUIBase.cpp
	Created: 11 Dec 2016 1:22:27pm
	Author:  Ben

  ==============================================================================
*/

AutomationKeyTimelineUIBase::AutomationKeyTimelineUIBase(AutomationKeyBase * key) :
	BaseItemMinimalUI(key),
	keyYPos1(-1),
    keyYPos2(-1),
	selectedHandleIndex(-1),
    showHandle(true)
{
	for (int i = 0; i < key->numDimensions; i++)
	{
		Handle * h = new Handle(dimensionColors[i]);
		handles.add(h);
		addAndMakeVisible(h);
	}
	//removeMouseListener(this);

	bringToFrontOnSelect = false;
	setWantsKeyboardFocus(false);
	setMouseClickGrabsKeyboardFocus(false);

	autoDrawContourWhenSelected = false;
	setEasingUI(item->easingBase != nullptr ? item->easingBase->createUI() : nullptr);
}

AutomationKeyTimelineUIBase::~AutomationKeyTimelineUIBase()
{
}

void AutomationKeyTimelineUIBase::setShowHandle(bool value)
{

	if (showHandle == value) return;
	showHandle = value;
	
	for (auto& handle : handles)
	{
		if (showHandle)
		{
			if (handle->getParentComponent() == nullptr) addChildComponent(handle);
		}
		else
		{
			if (handle->getParentComponent() == this) removeChildComponent(handle);
		}
	}
	
}


void AutomationKeyTimelineUIBase::setEasingUI(EasingUI * eui)
{
	if (easingUI != nullptr)
	{
		removeChildComponent(easingUI.get());
	}

	easingUI.reset(eui);

	if (easingUI != nullptr)
	{

		addAndMakeVisible(easingUI.get());
		easingUI->toBack();
		resized();
		if(keyYPos1 > -1 && keyYPos2 > -1) easingUI->setKeyPositions(keyYPos1, keyYPos2);
	}
}

void AutomationKeyTimelineUIBase::setKeyPositions(const int &k1, const int &k2)
{
	keyYPos1 = k1;
	keyYPos2 = k2;
	if (easingUI != nullptr) easingUI->setKeyPositions(keyYPos1, keyYPos2);

	Array<float> values = item->getValues();
	for (int i = 0; i < item->numDimensions; i++)
	{
		juce::Rectangle<int> hr = getLocalBounds().withSize(AutomationKeyTimelineUIBase::handleClickZone, AutomationKeyTimelineUIBase::handleClickZone)
			.withCentre(Point<int>(AutomationKeyTimelineUIBase::handleClickZone / 2, (int)((1 - values[i]) * getHeight())));

		handles[i]->setBounds(hr);
	}
}

void AutomationKeyTimelineUIBase::showKeyEditorWindow()
{
	AlertWindow keyEditorWindow("Set key position and value", "Fine tune this key's position and value", AlertWindow::AlertIconType::NoIcon, this);
	keyEditorWindow.addTextEditor("pos", item->position->stringValue(), "Position");
	keyEditorWindow.addTextEditor("val", item->value->stringValue(), "Value");


	keyEditorWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	keyEditorWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = keyEditorWindow.runModalLoop();

	if (result)
	{
		float newPos = keyEditorWindow.getTextEditorContents("pos").getFloatValue();
		float newValue = keyEditorWindow.getTextEditorContents("val").getFloatValue();

		Array<UndoableAction *> actions;
		actions.add(item->position->setUndoableValue(item->position->value,newPos,true));
		actions.add(item->value->setUndoableValue(item->value->value, newValue, true));
		UndoMaster::getInstance()->performActions("Move automation key", actions);
	}
}

void AutomationKeyTimelineUIBase::resized()
{
	Array<float> values = item->getValues();
	for (int i = 0; i < item->numDimensions; i++)
	{
		juce::Rectangle<int> hr = getLocalBounds().withSize(AutomationKeyTimelineUIBase::handleClickZone, AutomationKeyTimelineUIBase::handleClickZone)
			.withCentre(Point<int>(AutomationKeyTimelineUIBase::handleClickZone / 2, (int)((1 - values[i]) * getHeight())));

		handles[i]->setBounds(hr);
	}

	juce::Rectangle<int> r = getLocalBounds().reduced(AutomationKeyTimelineUIBase::handleClickZone / 2, 0);
	if (easingUI != nullptr)
	{
		easingUI->setBounds(r);
	}
}

bool AutomationKeyTimelineUIBase::hitTest(int tx, int ty)
{
	for (auto& handle : handles)
	{
		if (handle->getBounds().contains(tx, ty)) return true;
	}
	
	if (easingUI != nullptr && easingUI->hitTest(tx, ty)) return true;

	return false;
}

void AutomationKeyTimelineUIBase::mouseDown(const MouseEvent & e)
{
	BaseItemMinimalUI::mouseDown(e);
	setMouseCursor(e.mods.isShiftDown() ? MouseCursor::LeftRightResizeCursor : MouseCursor::NormalCursor);
	selectedHandleIndex = -1;
	for (int i = 0; i < item->numDimensions; i++)
	{
		if (e.eventComponent == handles[i])
		{
			if (e.mods.isLeftButtonDown())
			{
				if (e.mods.isCommandDown())
				{
					//add to selection here
				}
				else
				{
					posAtMouseDown = item->position->floatValue();
					valueAtMouseDown = item->value->floatValue();
					selectedHandleIndex = i;
				}
			}
		}
	}

	if (e.eventComponent == easingUI.get())
	{
		if (e.mods.isRightButtonDown())
		{
			PopupMenu ep;
			juce::StringArray keys = item->easingType->getAllKeys();
			int kid = 1;
			for (auto &i : keys)
			{
				ep.addItem(kid, i, true, i == item->easingType->getValueKey());
				kid++;
			}

			int result = ep.show();
			if (result >= 1 && result <= keys.size())
			{
				item->easingType->setUndoableValue(item->easingType->value,keys[result - 1]);
				item->easingBase->selectThis(); //reselect after changing easing
			}
		} else if (e.mods.isCommandDown())
		{
			item->easingType->setNext(true,true);
			item->easingBase->selectThis(); //reselect after changing easing
		}
	}
}

void AutomationKeyTimelineUIBase::mouseUp(const MouseEvent & e)
{
	selectedHandleIndex = -1;
	//handle->setMouseCursor(MouseCursor::NormalCursor);
}

void AutomationKeyTimelineUIBase::controllableFeedbackUpdateInternal(Controllable * c)
{
	if (c == item->easingType)
	{
		setEasingUI(item->easingBase != nullptr ? item->easingBase->createUI() : nullptr);
	}
}

void AutomationKeyTimelineUIBase::inspectableSelectionChanged(Inspectable * i)
{
	BaseItemMinimalUI::inspectableSelectionChanged(i);
	for (int i = 0; i < item->numDimensions; i++)
	{
		handles[i]->highlight = item->isSelected && selectedHandleIndex == i;
		handles[i]->repaint();
	}
}

void AutomationKeyTimelineUIBase::inspectablePreselectionChanged(Inspectable * i)
{
	BaseItemMinimalUI::inspectablePreselectionChanged(i);
	for (int i = 0; i < item->numDimensions; i++)
	{
		handles[i]->highlight = false;
		handles[i]->repaint();
	}
}

AutomationKeyTimelineUIBase::Handle::Handle(Colour c) :
	highlight(false),
	color(c)
{
	setRepaintsOnMouseActivity(true);
	setWantsKeyboardFocus(false);
	setMouseClickGrabsKeyboardFocus(false);
}

void AutomationKeyTimelineUIBase::Handle::paint(Graphics & g)
{
	Colour c = highlight ? HIGHLIGHT_COLOR : /*item->isPreselected ? PRESELECT_COLOR : */color;

	int rad = AutomationKeyTimelineUIBase::handleSize;
	if (isMouseOver() || highlight) rad += 4;

	juce::Rectangle<float> er = getLocalBounds().withSizeKeepingCentre(rad, rad).toFloat();

	Colour cc = isMouseOver() ? c.brighter() : c.darker(.3f);
	g.setColour(c);
	g.fillEllipse(er);
	g.setColour(cc);
	g.drawEllipse(er, 1);
}