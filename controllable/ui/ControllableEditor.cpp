/*
  ==============================================================================

	ControllableEditor.cpp
	Created: 7 Oct 2016 2:04:37pm
	Author:  bkupe

  ==============================================================================
*/


ControllableEditor::ControllableEditor(Array<Controllable*> controllables, bool isRoot) :
	InspectableEditor(Inspectable::getArrayAs<Controllable, Inspectable>(controllables), isRoot),
	controllables(Inspectable::getWeakArray(controllables)),
	controllable(controllables[0]),
	label("Label"),
	subContentHeight(0),
	minLabelWidth(160),
	minControlUIWidth(100),
	dragAndDropEnabled(true),
	showLabel(true)
{
	buildControllableUI();

	addAndMakeVisible(&label);
	label.setJustificationType(Justification::left);
	label.setBorderSize(BorderSize<int>(0));
	label.setFont(label.getFont().withHeight(GlobalSettings::getInstance()->fontSize->floatValue()));
	label.setText(controllable->niceName, dontSendNotification);
	label.setTooltip(ui->tooltip);
	if (!isMultiEditing())label.addMouseListener(this, false);

	bool isAllRemovable = multiHasAllOf([](Inspectable* i) {return ((Controllable*)i)->isRemovableByUser; });
	if (isAllRemovable)
	{
		removeBT.reset(AssetManager::getInstance()->getRemoveBT());
		removeBT->addListener(this);
		removeBT->setWantsKeyboardFocus(false);
		removeBT->setMouseClickGrabsKeyboardFocus(false);
		addAndMakeVisible(removeBT.get());
	}

	bool canAllDisable = multiHasAllOf([](Inspectable* i) {return ((Controllable*)i)->canBeDisabledByUser; });
	if (canAllDisable)
	{
		enableBT.reset(AssetManager::getInstance()->getPowerBT());
		enableBT->addListener(this);

		bool allEnabled = multiHasAllOf([](Inspectable* i) {return ((Controllable*)i)->enabled; });
		enableBT->setToggleState(allEnabled, dontSendNotification);
		addAndMakeVisible(enableBT.get());
	}

	bool showAllWarnings = multiHasAllOf([](Inspectable* i) {return ((Controllable*)i)->showWarningInUI; });
	if (showAllWarnings)
	{
		warningUI.reset(new WarningTargetUI(controllable));
		warningUI->addComponentListener(this);
		addChildComponent(warningUI.get());
	}

	baseHeight = ui->getHeight();
	if (baseHeight == 0) baseHeight = GlobalSettings::getInstance()->fontSize->floatValue() + 4;
	setSize(100, baseHeight);

	for (auto& c : controllables) c->addAsyncControllableListener(this);
}

ControllableEditor::~ControllableEditor()
{
	for (auto& c : controllables)
	{
		if (c.wasObjectDeleted()) continue;
		c->removeAsyncControllableListener(this);
	}

}

void ControllableEditor::setShowLabel(bool value)
{
	if (showLabel == value) return;
	showLabel = value;
	if (showLabel)
	{
		addAndMakeVisible(&label);
	}
	else
	{
		removeChildComponent(&label);
	}

}

void ControllableEditor::buildControllableUI(bool resizeAfter)
{
	if (ui != nullptr) removeChildComponent(ui.get());
	ui.reset(controllable->createDefaultUI(Inspectable::getArrayFromWeak(controllables)));
	ui->showLabel = false;
	ui->setOpaqueBackground(true);
	addAndMakeVisible(ui.get());

	if (resizeAfter) resized();
}

void ControllableEditor::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	r.removeFromBottom(subContentHeight);// .withHeight(16);

	int buttonSize = jmin(r.getHeight(), 20);

	if (warningUI != nullptr && warningUI->isVisible())
	{
		warningUI->setBounds(r.removeFromLeft(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize)); //warning
		r.removeFromLeft(2);
	}

	if (!controllable.wasObjectDeleted())
	{
		if (removeBT != nullptr)
		{
			removeBT->setBounds(r.removeFromRight(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
			r.removeFromRight(2);
		}


		if (enableBT != nullptr)
		{
			enableBT->setBounds(r.removeFromLeft(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));
			r.removeFromLeft(2);
		}
	}

	resizedInternal(r);

	int controlSpace = jmax<int>(showLabel ? jmin<int>(getWidth() - minLabelWidth, getWidth() * .75f) : getWidth(), minControlUIWidth);

	ui->setBounds(r.removeFromRight(controlSpace));

	if (showLabel)
	{
		r.removeFromRight(2);
		label.setBounds(r);
	}

}

void ControllableEditor::mouseDown(const MouseEvent& e)
{
	InspectableEditor::mouseDown(e);

	if (e.mods.isRightButtonDown())
	{
		if (e.eventComponent == &label)
		{
			if (ui->showMenuOnRightClick) ui->showContextMenu();
		}
	}
}

void ControllableEditor::mouseDrag(const MouseEvent& e)
{
	InspectableEditor::mouseDrag(e);

	if (dragAndDropEnabled && e.eventComponent == &label)
	{
		var desc = var(new DynamicObject());
		desc.getDynamicObject()->setProperty("type", controllable->getTypeString());
		desc.getDynamicObject()->setProperty("dataType", "Controllable");
		//Image dragImage = this->createComponentSnapshot(this->getLocalBounds()).convertedToFormat(Image::ARGB);
		//dragImage.multiplyAllAlphas(.5f);
		Point<int> offset = -getMouseXYRelative();
		if (e.getDistanceFromDragStart() > 30) startDragging(desc, this, ScaledImage(), true, &offset);
	}

}

void ControllableEditor::newMessage(const Controllable::ControllableEvent& e)
{
	switch (e.type)
	{
	case Controllable::ControllableEvent::NAME_CHANGED:
		if (!isMultiEditing()) label.setText(controllable->niceName, dontSendNotification);
		break;

	case Controllable::ControllableEvent::STATE_CHANGED:
		if (enableBT != nullptr)
		{
			bool allEnabled = multiHasAllOf([](Inspectable* i) {return ((Controllable*)i)->enabled; });
			enableBT->setToggleState(allEnabled, dontSendNotification);
		}
		break;

	default:
		break;

	}
}

void ControllableEditor::componentVisibilityChanged(Component& c)
{
	InspectableEditor::componentVisibilityChanged(c);
	if (&c == warningUI.get()) resized();
}

void ControllableEditor::buttonClicked(Button* b)
{
	if (b == removeBT.get())
	{
		for (auto& c : controllables) c->remove(true); //would need proper grouped undo
	}
	else if (b == enableBT.get())
	{
		bool targetEnabled = controllables.size() > 0 ? !controllables[0]->enabled : false;
		for (auto& c : controllables) c->setEnabled(targetEnabled);
	}
}
