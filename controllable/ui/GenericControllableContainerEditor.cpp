/*
  ==============================================================================

	GenericControllableContainerEditor.cpp
	Created: 9 May 2016 6:41:59pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

GenericControllableContainerEditor::GenericControllableContainerEditor(Array<ControllableContainer*> containers, bool isRoot, bool buildAtCreation) :
	InspectableEditor(Inspectable::getArrayAs<ControllableContainer, Inspectable>(containers), isRoot),
	headerHeight(GlobalSettings::getInstance()->fontSize->floatValue() + 8),
	isRebuilding(false),
	prepareToAnimate(false),
	contourColor(BG_COLOR.brighter(.3f)),
	containerLabel("containerLabel", dynamic_cast<ControllableContainer*>(inspectable.get())->niceName),
	containers(containers),
	customCreateEditorForControllableFunc(nullptr),
	customCreateEditorForContainerFunc(nullptr),
	dragAndDropEnabled(false)
{
	jassert(containers.size() > 0);

	container = containers[0];
	container->addAsyncContainerListener(this);
	addAndMakeVisible(containerLabel);

	containerLabel.setFont(containerLabel.getFont().withHeight(GlobalSettings::getInstance()->fontSize->floatValue()));

	containerLabel.setColour(containerLabel.backgroundWhenEditingColourId, Colours::black);
	containerLabel.setColour(containerLabel.textWhenEditingColourId, Colours::white);
	containerLabel.setColour(containerLabel.textColourId, contourColor.brighter(1));
	containerLabel.setColour(CaretComponent::caretColourId, Colours::orange);

	containerLabel.setEditable(false, container->nameCanBeChangedByUser);
	containerLabel.addListener(this);

	if (!container->nameCanBeChangedByUser) containerLabel.setInterceptsMouseClicks(false, false);

	if (container->userCanAddControllables)
	{
		addBT.reset(AssetManager::getInstance()->getAddBT());
		addBT->addListener(this);
		addAndMakeVisible(addBT.get());
	}

	if (container->showWarningInUI)
	{
		warningUI.reset(new WarningTargetUI(container));
		addChildComponent(warningUI.get());
		warningUI->addComponentListener(this);
	}

	if (container->isRemovableByUser)
	{
		removeBT.reset(AssetManager::getInstance()->getRemoveBT());
		removeBT->addListener(this);
		removeBT->setWantsKeyboardFocus(false);
		removeBT->setMouseClickGrabsKeyboardFocus(false);

		addAndMakeVisible(removeBT.get());
	}

	//setDragAndDropEnabled(true);

	if (canBeCollapsed())
	{
		expandBT.reset(AssetManager::getInstance()->getRightArrowBT());
		collapseBT.reset(AssetManager::getInstance()->getDownArrowImageBT());
		expandBT->addListener(this);
		collapseBT->addListener(this);

		addChildComponent(expandBT.get());
		addChildComponent(collapseBT.get());

		expandBT->setWantsKeyboardFocus(false);
		collapseBT->setWantsKeyboardFocus(false);
		expandBT->setMouseClickGrabsKeyboardFocus(false);
		collapseBT->setMouseClickGrabsKeyboardFocus(false);

		collapseAnimator.addChangeListener(this);

		setCollapsed(container->editorIsCollapsed, true, false, !buildAtCreation);
	}
	else
	{
		if (buildAtCreation) resetAndBuild();
	}



}

GenericControllableContainerEditor::~GenericControllableContainerEditor()
{
	if (!inspectable.wasObjectDeleted())
	{
		if (container != nullptr) container->removeAsyncContainerListener(this);
		clear();
	}
}

void GenericControllableContainerEditor::clear()
{
	for (auto& c : childEditors) removeChildComponent(c);
	childEditors.clear();
}

void GenericControllableContainerEditor::mouseDown(const MouseEvent& e)
{
	//if (e.mods.isLeftButtonDown())
	//{
	//	if (e.originalComponent == &headerSpacer || (isRoot && e.eventComponent == this && e.getMouseDownY() < headerHeight))
	//	{
	//		if (e.mods.isShiftDown()) toggleCollapsedChildren();
	//		else setCollapsed(!container->editorIsCollapsed);
	//	}
	//}
	//else if (e.mods.isRightButtonDown())
	//{
	//	showContextMenu();
	//}
}

void GenericControllableContainerEditor::mouseUp(const MouseEvent& e)
{
	if (e.mods.isLeftButtonDown())
	{
		if (canBeCollapsed())
		{
			if ((e.eventComponent == this && e.getMouseDownPosition().y < headerHeight && !dragRect.contains(e.getMouseDownPosition())) || (isRoot && e.eventComponent == this && e.getMouseDownY() < headerHeight))
			{
				if (e.mods.isShiftDown()) toggleCollapsedChildren();
				else setCollapsed(!container->editorIsCollapsed);
			}
		}
	}
	else if (e.mods.isRightButtonDown())
	{
		showContextMenu();
	}
}

void GenericControllableContainerEditor::mouseDrag(const MouseEvent& e)
{
	InspectableEditor::mouseDrag(e);

	if (!dragAndDropEnabled) return;

	if (dragRect.contains(e.getMouseDownPosition()) || (e.eventComponent == this && e.getMouseDownPosition().y < headerHeight))
	{
		if (e.getDistanceFromDragStart() > 30)
		{
			Point<int> offset = -getMouseXYRelative();
			var desc = var(new DynamicObject());
			desc.getDynamicObject()->setProperty("type", "ControllableContainer");
			desc.getDynamicObject()->setProperty("dataType", "Container");
			setDragDetails(desc);
			startDragging(desc, this, ScaledImage(), true, &offset);
		}
	}
}

void GenericControllableContainerEditor::setDragAndDropEnabled(bool value)
{
	if (dragAndDropEnabled == value) return;

	dragAndDropEnabled = value;

	//for (auto& e : childEditors)
	//{
	//	GenericControllableContainerEditor* gce = dynamic_cast<GenericControllableContainerEditor*>(e);
	//	if (gce != nullptr) setDragAndDropEnabled(value);

	//	ControllableEditor* ce = dynamic_cast<ControllableEditor*>(e);
	//	if (ce != nullptr) ce->dragAndDropEnabled = value;
	//}

	resized();
}

void GenericControllableContainerEditor::showContextMenu()
{
	PopupMenu p;
	p.addItem(1, "Toggle childrens");
	if (container->canBeCopiedAndPasted)
	{
		p.addItem(2, "Copy");
		p.addItem(3, "Paste (replace)");
	}

	addPopupMenuItems(&p);

	if (ControllableUI::showDashboardOption)
	{
		p.addSeparator();
		PopupMenu dashboardMenu;
		int index = 0;
		for (auto& di : DashboardManager::getInstance()->items)
		{
			dashboardMenu.addItem(index + 10000, di->niceName);
			index++;
		}

		p.addSubMenu("Send to Dashboard", dashboardMenu);
	}


	p.addSeparator();

	if (ControllableUI::showOSCControlAddressOption) p.addItem(-1000, "Copy OSC Control Address");
	if (ControllableUI::showScriptControlAddressOption) p.addItem(-1001, "Copy Script Control Address");

	p.showMenuAsync(PopupMenu::Options(), [this](int result)
		{
			if (result != 0)
			{
				switch (result)
				{
				case 1:
					this->toggleCollapsedChildren();
					break;

				case 2:
					SystemClipboard::copyTextToClipboard(JSON::toString(this->container->getJSONData()));
					break;

				case 3:
					this->container->loadJSONData(JSON::fromString(SystemClipboard::getTextFromClipboard()));
					break;



				case -1000:
					SystemClipboard::copyTextToClipboard(this->container->getControlAddress());
					break;
				case -1001:
					SystemClipboard::copyTextToClipboard("root" + this->container->getControlAddress().replaceCharacter('/', '.'));
					break;

				default:
					if (result >= 10000)
					{
						DashboardManager::getInstance()->items[result - 10000]->itemManager.addItem(this->container->createDashboardItem());
					}
					else
					{
						this->handleMenuSelectedID(result);
					}
					break;
				}
			}
		}
	);
}


void GenericControllableContainerEditor::setCollapsed(bool value, bool force, bool animate, bool doNotRebuild)
{
	if (container.wasObjectDeleted()) return;
	if (container->editorIsCollapsed == value && !force) return;

	if (isRoot || !container->editorCanBeCollapsed) return;

	container->editorIsCollapsed = value;

	if (collapseBT != nullptr) collapseBT->setVisible(!container->editorIsCollapsed);
	if (expandBT != nullptr) expandBT->setVisible(container->editorIsCollapsed);

	if (animate) prepareToAnimate = true;
	int targetHeight = headerHeight;

	if (!container->editorIsCollapsed)
	{
		if (!doNotRebuild) resetAndBuild();
		juce::Rectangle<int> r = getLocalBounds();

		if (canBeCollapsed())
		{
			resizedInternal(r);
			targetHeight = jmax<int>(r.getY() + 6, headerHeight);
		}
	}

	if (animate)
	{
		prepareToAnimate = false;
		if (isVisible()) collapseAnimator.animateComponent(this, getBounds().withHeight(targetHeight), 1, 200, false, 1, 0);
	}
	else
	{
		setSize(getWidth(), targetHeight);
	}

}

void GenericControllableContainerEditor::toggleCollapsedChildren()
{
	Array< GenericControllableContainerEditor*> childContainers;
	bool shouldCollapse = false;
	for (auto& cui : childEditors)
	{
		if (GenericControllableContainerEditor* ccui = dynamic_cast<GenericControllableContainerEditor*>(cui))
		{
			childContainers.add(ccui);
			if (ccui->container->editorCanBeCollapsed && !ccui->container->editorIsCollapsed) shouldCollapse = true;
		}
	}

	for (auto& cui : childContainers) cui->setCollapsed(shouldCollapse);
}

void GenericControllableContainerEditor::resetAndBuild()
{
	isRebuilding = true;

	clear();

	if (container == nullptr)
	{
		LOGWARNING("An error has occured here (container null on ResetAndBuild");
		return;
	}
	//if (container->hideInEditor) return;

	if (!canBeCollapsed() || !container->editorIsCollapsed)
	{
		if (container->controllables.getLock().tryEnter())
		{
			for (auto& c : container->controllables)
			{
				if (c == nullptr)
				{
					LOGWARNING("An error has occured here (child controllable null on ResetAndBuild");
					continue;

				}

				if (!c->hideInEditor) addControllableUI(c);
			}
			container->controllables.getLock().exit();
		}


		if (container->canInspectChildContainers)
		{
			if (container->controllableContainers.getLock().tryEnter())
			{
				for (auto& cc : container->controllableContainers)
				{
					if (cc.wasObjectDeleted() || cc == nullptr)
					{
						LOGWARNING("An error has occured here (child container null on ResetAndBuild");
						continue;
					}

					if (!cc->hideInEditor) addEditorUI(cc);
				}

				container->controllableContainers.getLock().exit();
			}

		}
	}

	isRebuilding = false;
	resized();

	containerEditorListeners.call(&ContainerEditorListener::containerRebuilt, this);
}

InspectableEditor* GenericControllableContainerEditor::getEditorUIForContainer(ControllableContainer* cc)
{
	if (customCreateEditorForContainerFunc != nullptr) return customCreateEditorForContainerFunc(container, cc);
	return cc->getEditor(false);
}

InspectableEditor* GenericControllableContainerEditor::addEditorUI(ControllableContainer* cc, bool resize)
{
	if (cc == nullptr) return nullptr;

	InspectableEditor* ccui = getEditorUIForContainer(cc);

	//GenericControllableContainerEditor* gce = dynamic_cast<GenericControllableContainerEditor*>(ccui);
	//if (gce != nullptr) gce->setDragAndDropEnabled(dragAndDropEnabled);

	int index = container->controllableContainers.indexOf(cc);
	childEditors.insert(container->controllables.size() + index, ccui);
	addAndMakeVisible(ccui);
	if (resize) resized();
	return ccui;
}

void GenericControllableContainerEditor::removeEditorUI(InspectableEditor* ccui, bool resize)
{
	if (ccui == nullptr)
	{
		resetAndBuild();
		return;
	}

	removeChildComponent(ccui);
	childEditors.removeObject(ccui);
	if (resize) resized();
}

void GenericControllableContainerEditor::showMenuAndAddControllable()
{
	ControllableFactory::showFilteredCreateMenu(container->userAddControllablesFilters, [this](Controllable* c)
		{
			c->userCanChangeName = true;
			c->isCustomizableByUser = true;
			c->isRemovableByUser = true;
			c->isSavable = true;
			c->saveValueOnly = false;
			this->container->addControllable(c);
		}
		, true
			);

}

InspectableEditor* GenericControllableContainerEditor::getEditorForInspectable(Inspectable* i)
{
	for (auto& cui : childEditors)
	{
		if (cui->inspectable == i) return cui;
	}

	return nullptr;
}

void GenericControllableContainerEditor::buttonClicked(Button* b)
{
	if (b == expandBT.get()) setCollapsed(false);
	else if (b == collapseBT.get()) setCollapsed(true);
	else if (b == addBT.get())
	{
		if (container->customUserCreateControllableFunc != nullptr) container->customUserCreateControllableFunc(container);
		else showMenuAndAddControllable();
	}
	else if (b == removeBT.get())
	{
		if (container->parentContainer != nullptr) container->parentContainer->removeChildControllableContainer(container);
	}
}

void GenericControllableContainerEditor::labelTextChanged(Label* l)
{
	if (l == &containerLabel) container->setUndoableNiceName(l->getText());
}

void GenericControllableContainerEditor::componentVisibilityChanged(Component& c)
{
	if (container.wasObjectDeleted()) return;
	if (&c == warningUI.get()) resized();
}

InspectableEditor* GenericControllableContainerEditor::getEditorUIForControllable(Controllable* c)
{
	if (customCreateEditorForControllableFunc != nullptr) return customCreateEditorForControllableFunc(container, c);
	return c->getEditor(false);
}

InspectableEditor* GenericControllableContainerEditor::addControllableUI(Controllable* c, bool resize)
{
	if (c == nullptr || c->hideInEditor) return nullptr;

	InspectableEditor* cui = getEditorUIForControllable(c);

	//ControllableEditor* ce = dynamic_cast<ControllableEditor*>(cui);
	//if (ce != nullptr) ce->dragAndDropEnabled = dragAndDropEnabled;

	int index = container->controllables.indexOf(c);
	childEditors.insert(index, cui);
	addAndMakeVisible(cui);
	if (resize) resized();
	return nullptr;// cui;
}

void GenericControllableContainerEditor::removeControllableUI(Controllable* c, bool resize)
{
	InspectableEditor* cui = getEditorForInspectable(c);
	if (cui == nullptr) return;

	removeChildComponent(cui);
	childEditors.removeObject(cui);
	if (resize) resized();

}

void GenericControllableContainerEditor::newMessage(const ContainerAsyncEvent& p)
{
	switch (p.type)
	{
	case ContainerAsyncEvent::ControllableAdded:
		//if (p.targetControllable->parentContainer != container) return;
		//if (p.targetControllable->hideInEditor) return;
		//addControllableUI(p.targetControllable, true);
		resetAndBuild();
		//resized();
		break;

	case ContainerAsyncEvent::ControllableRemoved:
		removeControllableUI(p.targetControllable, true);
		//resetAndBuild();
		//resized();
		break;

	case ContainerAsyncEvent::ControllableContainerAdded:
		resetAndBuild();
		//resized();
		break;

	case ContainerAsyncEvent::ControllableContainerRemoved:
		removeEditorUI(getEditorForInspectable(p.targetContainer), true);
		break;

	case ContainerAsyncEvent::ChildStructureChanged:
		//nothing ?
		break;

	case ContainerAsyncEvent::ControllableContainerReordered:
		resetAndBuild();
		//resized();
		break;

	case ContainerAsyncEvent::ControllableFeedbackUpdate:
		controllableFeedbackUpdate(p.targetControllable);
		break;

	case ContainerAsyncEvent::ChildAddressChanged:
		containerLabel.setText(container->niceName, dontSendNotification);
		resized();
		break;

	case ContainerAsyncEvent::ControllableContainerNeedsRebuild:
		resetAndBuild();
		//resized();
		break;

	case ContainerAsyncEvent::ControllableContainerCollapsedChanged:
		setCollapsed(container->editorIsCollapsed, true);
		break;

	default:
		//not handled
		break;

	}
}

void GenericControllableContainerEditor::childBoundsChanged(Component* c)
{
	if (isRebuilding) return;
	if (getWidth() == 0 || getHeight() == 0) return;

	resized();

}

void GenericControllableContainerEditor::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &collapseAnimator)
	{
		if (!collapseAnimator.isAnimating() && container->editorIsCollapsed) resetAndBuild();
	}
}


void GenericControllableContainerEditor::paint(Graphics& g)
{
	if (container == nullptr)
	{
		DBG("Container is null !");
		resetAndBuild();
		return;
	}

	/*
	if (!isRoot && parentInspector != nullptr)
	{
		juce::Rectangle<int> r = getLocalBounds().getIntersection(getLocalArea(parentInspector, parentInspector->getLocalBounds()));
		g.setColour(Colours::orange.withAlpha(.2f));
		g.fillRect(r.reduced(10));
	}
	*/

	if (!isRoot && !container->hideEditorHeader)
	{
		g.setColour(contourColor.withMultipliedAlpha(.3f));
		juce::Rectangle<int> r = getLocalBounds();
		if (container->editorIsCollapsed && container->editorCanBeCollapsed) r.setHeight(headerHeight);
		g.fillRoundedRectangle(r.toFloat(), 4);
	}

	if ((isRoot || canBeCollapsed()) /*&& !container->hideEditorHeader*/)
	{
		g.setColour(contourColor.withMultipliedAlpha(isRoot ? .8f : .4f));
		g.fillRoundedRectangle(getHeaderBounds().toFloat(), 4);
	}

	if (!isRoot && !container->hideEditorHeader)
	{
		g.setColour(contourColor.brighter(.2f));
		g.drawRoundedRectangle(getLocalBounds().toFloat(), 4, 2);
	}

	if (dragAndDropEnabled)
	{
		g.setColour(Colours::white);
		g.drawImage(AssetManager::getInstance()->drag, dragRect.toFloat());
	}

}

void GenericControllableContainerEditor::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	isRebuilding = true;
	resizedInternal(r);
	isRebuilding = false;
	if (!container.wasObjectDeleted() && !collapseAnimator.isAnimating() && !prepareToAnimate) setSize(getWidth(), (!isRoot && container->editorIsCollapsed) ? headerHeight : jmax<int>(r.getY() + 2, headerHeight));
}

void GenericControllableContainerEditor::resizedInternal(juce::Rectangle<int>& r)
{
	if (container == nullptr)
	{
		DBG("Container is null !");
		resetAndBuild();
		return;
	}

	if (isRoot || !container->hideEditorHeader)
	{
		juce::Rectangle<int> hr = r.removeFromTop(headerHeight);

		if (canBeCollapsed()) //draw arrow here to have better control in resizedInternalHeader overrides
		{
			juce::Rectangle<int> ar = hr.removeFromLeft(headerHeight).reduced(4);
			if (container->editorIsCollapsed) expandBT->setBounds(ar);
			else collapseBT->setBounds(ar);
			r.removeFromLeft(2);
		}

		if (addBT != nullptr)
		{
			addBT->setBounds(hr.removeFromRight(headerHeight));
			hr.removeFromRight(2);
		}

		resizedInternalHeader(hr);
		r.removeFromTop(headerGap);
	}

	r.reduce(2, 2);
	if (!isRoot && !container->hideEditorHeader) r.removeFromLeft(4);
	if (!canBeCollapsed() || !container->editorIsCollapsed) resizedInternalContent(r);
}

void GenericControllableContainerEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (container->isRemovableByUser && removeBT != nullptr)
	{
		removeBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
		r.removeFromRight(2);
	}

	if (dragAndDropEnabled)
	{
		dragRect = r.removeFromRight(r.getHeight()).reduced(4);
	}

	if (warningUI != nullptr && warningUI->isVisible())
	{
		warningUI->setBounds(r.removeFromLeft(r.getHeight()).reduced(2));
		r.removeFromLeft(2);
	}

	if (containerLabel.isVisible()) containerLabel.setBounds(r.removeFromLeft(containerLabel.getFont().getStringWidth(containerLabel.getText()) + 20));
}

void GenericControllableContainerEditor::resizedInternalContent(juce::Rectangle<int>& r)
{
	for (auto& cui : childEditors)
	{
		if (cui->inspectable.wasObjectDeleted()) continue;
		if (cui->isVisible() == cui->inspectable->hideInEditor) cui->setVisible(!cui->inspectable->hideInEditor);
		if (cui->inspectable->hideInEditor) continue;

		int th = cui->getHeight();
		cui->setBounds(r.withHeight(th));

		float gap = 4;
		if (isRoot && dynamic_cast<GenericControllableContainerEditor*>(cui) != nullptr) gap = 16;

		r.translate(0, th + gap);
	}
}


juce::Rectangle<int> GenericControllableContainerEditor::getHeaderBounds()
{
	return getLocalBounds().withHeight(headerHeight);
}

juce::Rectangle<int> GenericControllableContainerEditor::getContentBounds()
{
	return getLocalBounds().withTop(headerHeight + headerGap).reduced(2);
}

bool GenericControllableContainerEditor::canBeCollapsed()
{
	return !container.wasObjectDeleted() && !isRoot && container->editorCanBeCollapsed && !container->hideEditorHeader;
}



//EnablingControllableContainerEditor

EnablingControllableContainerEditor::EnablingControllableContainerEditor(Array<EnablingControllableContainer*> cc, bool isRoot, bool buildAtCreation) :
	GenericControllableContainerEditor(Inspectable::getArrayAs<EnablingControllableContainer, ControllableContainer>(cc), isRoot, buildAtCreation),
	ioContainers(cc)
{
	jassert(ioContainers.size() > 0);

	ioContainer = cc[0];

	if (ioContainer->canBeDisabled)
	{
		enabledUI.reset(ioContainer->enabled->createToggle(ImageCache::getFromMemory(OrganicUIBinaryData::power_png, OrganicUIBinaryData::power_pngSize)));
		addAndMakeVisible(enabledUI.get());
	}
}

void EnablingControllableContainerEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (((EnablingControllableContainer*)container.get())->canBeDisabled) enabledUI->setBounds(r.removeFromLeft(r.getHeight()).reduced(2));
	GenericControllableContainerEditor::resizedInternalHeader(r);
}

void EnablingControllableContainerEditor::controllableFeedbackUpdate(Controllable* c)
{
	if (c == nullptr) return;

	if (c == ioContainer->enabled)
	{
		setCollapsed(!ioContainer->enabled->boolValue(), true);
	}
}
