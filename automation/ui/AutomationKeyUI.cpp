/*
  ==============================================================================

	AutomationKeyUI.cpp
	Created: 11 Dec 2016 1:22:27pm
	Author:  Ben

  ==============================================================================
*/


AutomationKeyUI::AutomationKeyUI(AutomationKey * key) :
	BaseItemMinimalUI(key),
	keyYPos1(-1),
    keyYPos2(-1),
    showHandles(true),
	draggingHandle(nullptr)
{
	
	for (int i = 0; i < key->numDimensions; i++)
	{
		Handle* h = new Handle(i, key->numDimensions == 1 ? Colours::white : Colour::fromHSV(i * .3f, .9f, 1, 0xFF));
		addAndMakeVisible(h);
		handles.add(h);
		easingsUI.add(nullptr);
	}

	//removeMouseListener(this);

	dragAndDropEnabled = false;
	bringToFrontOnSelect = false;
	setWantsKeyboardFocus(false);
	setMouseClickGrabsKeyboardFocus(false);

	autoDrawContourWhenSelected = false;
	updateEasingsUI();
}

AutomationKeyUI::~AutomationKeyUI()
{
}

void AutomationKeyUI::setShowHandles(bool value)
{

	if (showHandles == value) return;
	showHandles = value;

	if (showHandles)
	{
		for(int i=0;i<item->numDimensions;i++) if(handles[i]->getParentComponent() == nullptr) addChildComponent(handles[i]);
	} else
	{
		for (int i = 0; i < item->numDimensions; i++) if (handles[i]->getParentComponent() == this) removeChildComponent(handles[i]);
	}
}


void AutomationKeyUI::paint(Graphics& g)
{
	//nothing
}

void AutomationKeyUI::updateEasingsUI()
{
	for (int i = 0; i < item->numDimensions; i++)
	{
		if (easingsUI[i] != nullptr) removeChildComponent(easingsUI[i]);

		EasingUI* eui = item->easings[i] != nullptr ? item->easings[i]->createUI() : nullptr;
		easingsUI.set(i, eui);

		if (eui != nullptr)
		{

			addAndMakeVisible(eui);
			eui->color = item->numDimensions == 1 ? Colours::white : Colour::fromHSV(i * .3f, .9f, 1, 0xFF);
			eui->toBack();
			resized();
			eui->setKeyPositions(keyYPos1[i], keyYPos2[i]);
		}
	}
}

void AutomationKeyUI::setKeyPositions(const Array<int> k1, const Array<int> k2)
{
	keyYPos1 = k1;
	keyYPos2 = k2;

	for (int i = 0; i < item->numDimensions; i++)
	{
		if (easingsUI[i] != nullptr) easingsUI[i]->setKeyPositions(keyYPos1[i], keyYPos2[i]);
	}

	resized();
}

void AutomationKeyUI::showKeyEditorWindow(int index)
{
	AlertWindow keyEditorWindow("Set key position and value", "Fine tune this key's position and value", AlertWindow::AlertIconType::NoIcon, this);
	keyEditorWindow.addTextEditor("pos", item->position->stringValue(), "Position");
	/*for(int i=0;i<item->numDimensions;i++) */ keyEditorWindow.addTextEditor("val"/*+String(i)*/, item->values[index]->stringValue(), "Value" /*+ String(i)*/);


	keyEditorWindow.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	keyEditorWindow.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	int result = keyEditorWindow.runModalLoop();

	if (result)
	{
		float newPos = keyEditorWindow.getTextEditorContents("pos").getFloatValue();

		Array<UndoableAction *> actions;
		actions.add(item->position->setUndoableValue(item->position->value,newPos,true));
		//for (int i = 0; i < item->numDimensions; i++)
		//{
			float newValue = keyEditorWindow.getTextEditorContents("val"/*+String(i)*/).getFloatValue();
			actions.add(item->values[index]->setUndoableValue(item->values[index]->value, newValue, true));
		//}
		UndoMaster::getInstance()->performActions("Move automation key", actions);
	}
}

void AutomationKeyUI::resized()
{
	for (int i = 0; i < item->numDimensions; i++)
	{
		juce::Rectangle<int> hr = getLocalBounds().withSize(AutomationKeyUI::handleClickZone, AutomationKeyUI::handleClickZone)
			.withCentre(Point<int>(AutomationKeyUI::handleClickZone / 2, keyYPos1[i]));

		handles[i]->setBounds(hr);

		juce::Rectangle<int> r = getLocalBounds().reduced(AutomationKeyUI::handleClickZone / 2, 0);
		if (easingsUI[i] != nullptr) easingsUI[i]->setBounds(r);
	}
	
}

bool AutomationKeyUI::hitTest(int tx, int ty)
{
	for (int i = 0; i < item->numDimensions; i++)
	{
		if (handles[i]->getBounds().contains(tx, ty) || (easingsUI[i] != nullptr && easingsUI[i]->hitTest(tx, ty))) return true;
	}

	return false;
}

void AutomationKeyUI::mouseDown(const MouseEvent & e)
{
	BaseItemMinimalUI::mouseDown(e);
	setMouseCursor(e.mods.isShiftDown() ? MouseCursor::LeftRightResizeCursor : MouseCursor::NormalCursor);
	if (Handle * h = dynamic_cast<Handle *>(e.eventComponent))
	{
		if (e.mods.isLeftButtonDown())
		{
			if (e.mods.isCommandDown())
			{
				//add to selection here
			} else
			{
				posAtMouseDown = item->position->floatValue();
				valueAtMouseDown = item->values[h->dimensionIndex]->floatValue();
				draggingHandle = h;
			}
		}
	} else if (EasingUI * eui = dynamic_cast<EasingUI *>(e.eventComponent))
	{
		int index = easingsUI.indexOf(eui);
		jassert(index != -1);

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
				item->easings[index]->selectThis(); //reselect after changing easing
			}
		} else if (e.mods.isCommandDown())
		{
			item->easingType->setNext(true,true);
			item->easings[index]->selectThis(); //reselect after changing easing
		}
	}
}

void AutomationKeyUI::mouseDoubleClick(const MouseEvent& e)
{
	if (Handle* h = dynamic_cast<Handle*>(e.eventComponent))
	{
		showKeyEditorWindow(h->dimensionIndex);
	}
}

void AutomationKeyUI::mouseUp(const MouseEvent & e)
{
	if(draggingHandle != nullptr ) draggingHandle->setMouseCursor(MouseCursor::NormalCursor);
	draggingHandle = nullptr;
}

void AutomationKeyUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	if (c == item->easingType)
	{
		updateEasingsUI();
	}
}

void AutomationKeyUI::inspectableSelectionChanged(Inspectable * i)
{
	BaseItemMinimalUI::inspectableSelectionChanged(i);
	for (auto& h : handles)
	{
		h->highlight = item->isSelected;
		//h->color = item->isPreselected ? PRESELECT_COLOR : color;
	}
}

void AutomationKeyUI::inspectablePreselectionChanged(Inspectable * i)
{
	BaseItemMinimalUI::inspectablePreselectionChanged(i);
	for (auto& h : handles)
	{
		h->highlight = item->isSelected;
		//h->color = item->isPreselected ? PRESELECT_COLOR : color;
	}
}

AutomationKeyUI::Handle::Handle(int dimensionIndex, Colour c) :
	highlight(false),
	dimensionIndex(dimensionIndex),
	color(c)
{
	setRepaintsOnMouseActivity(true);
	setWantsKeyboardFocus(false);
	setMouseClickGrabsKeyboardFocus(false);
}

void AutomationKeyUI::Handle::paint(Graphics & g)
{

	int rad = AutomationKeyUI::handleSize;
	if (isMouseOver() || highlight) rad += 4;

	juce::Rectangle<float> er = getLocalBounds().withSizeKeepingCentre(rad, rad).toFloat();

	Colour cc = isMouseOver() ? color.brighter() : color.darker(.3f);
	g.setColour(color);
	g.fillEllipse(er);
	g.setColour(cc);
	g.drawEllipse(er, 1);
}

