#include "GenericControllableContainerEditor.h"
/*
  ==============================================================================

	GenericControllableContainerEditor.cpp
	Created: 9 May 2016 6:41:59pm
	Author:  bkupe

  ==============================================================================
*/


//ControllableUIComparator CCInnerContainer::comparator;

GenericControllableContainerEditor::GenericControllableContainerEditor(WeakReference<Inspectable> inspectable, bool isRoot, bool buildAtCreation) :
	InspectableEditor(inspectable, isRoot),
	headerHeight(18),
	isRebuilding(false),
	prepareToAnimate(false),
	contourColor(BG_COLOR.brighter(.3f)),
	containerLabel("containerLabel", dynamic_cast<ControllableContainer *>(inspectable.get())->niceName),
	container(dynamic_cast<ControllableContainer *>(inspectable.get())),
	headerSpacer("headerSpacer")
{
	container->addAsyncContainerListener(this);
	addAndMakeVisible(containerLabel);
	
	containerLabel.setFont(containerLabel.getFont().withHeight(headerHeight-6));

	containerLabel.setColour(containerLabel.backgroundWhenEditingColourId, Colours::black);
	containerLabel.setColour(containerLabel.textWhenEditingColourId, Colours::white);
	containerLabel.setColour(containerLabel.textColourId, contourColor.brighter(1));
	containerLabel.setColour(CaretComponent::caretColourId, Colours::orange);

	containerLabel.setEditable(false,container->nameCanBeChangedByUser);
	containerLabel.addListener(this);

	if(!container->nameCanBeChangedByUser) containerLabel.setInterceptsMouseClicks(false, false);

	if (canBeCollapsed())
	{
		expandBT = AssetManager::getInstance()->getRightArrowBT();
		collapseBT = AssetManager::getInstance()->getDownArrowImageBT();
		expandBT->addListener(this);
		collapseBT->addListener(this);

		addChildComponent(expandBT);
		addChildComponent(collapseBT);

		addAndMakeVisible(headerSpacer);
		headerSpacer.addMouseListener(this, false);

		collapseAnimator.addChangeListener(this);

		setCollapsed(container->editorIsCollapsed, true, false, !buildAtCreation);
	} else
	{
		if(buildAtCreation) resetAndBuild();
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
	for (auto &c : childEditors) removeChildComponent(c);
	childEditors.clear();
}

void GenericControllableContainerEditor::mouseDown(const MouseEvent & e)
{
	if (e.mods.isLeftButtonDown())
	{
		if (e.originalComponent == &headerSpacer) setCollapsed(!container->editorIsCollapsed);
	}
}


void GenericControllableContainerEditor::setCollapsed(bool value, bool force, bool animate, bool doNotRebuild)
{
	if (container->editorIsCollapsed == value && !force) return;
	
	if (isRoot || !container->editorCanBeCollapsed) return;

	container->editorIsCollapsed = value;
	
	collapseBT->setVisible(!container->editorIsCollapsed);
	expandBT->setVisible(container->editorIsCollapsed);
	
	if(animate) prepareToAnimate = true;
	int targetHeight = headerHeight;
	
	if (!container->editorIsCollapsed)
	{
		if(!doNotRebuild) resetAndBuild();
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
	} else
	{
		setSize(getWidth(),targetHeight);
	}

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
	if (container->hideInEditor) return;
	
	if (!canBeCollapsed() || !container->editorIsCollapsed)
	{
		
		for (auto &c : container->controllables)
		{
			if (c == nullptr)
			{
				LOGWARNING("An error has occured here (child controllable null on ResetAndBuild");
				continue;

			}

			if (!c->hideInEditor) addControllableUI(c);
		}
		
		if (container->canInspectChildContainers)
		{
			for (auto &cc : container->controllableContainers)
			{
				if (cc == nullptr)
				{
					LOGWARNING("An error has occured here (child container null on ResetAndBuild");
					continue;
				}

				if (!cc->hideInEditor) addEditorUI(cc);
			}
		}
	}

	isRebuilding = false;
	resized();
}

InspectableEditor * GenericControllableContainerEditor::addEditorUI(ControllableContainer * cc, bool resize)
{
	InspectableEditor * ccui = cc->getEditor(false);
	int index = container->controllableContainers.indexOf(cc);
	childEditors.insert(container->controllables.size()+index, ccui);
	addAndMakeVisible(ccui);
	if (resize) resized();
	return ccui;
}

void GenericControllableContainerEditor::removeEditorUI(ControllableContainer * cc, bool resize)
{
	InspectableEditor * ccui = getEditorForInspectable(cc);
	if (ccui == nullptr)
	{
		resetAndBuild();
		return;
	}

	removeChildComponent(ccui);
	childEditors.removeObject(ccui);
	if (resize) resized();
}

InspectableEditor * GenericControllableContainerEditor::getEditorForInspectable(Inspectable * i)
{
	for (auto &cui : childEditors)
	{
		if (cui->inspectable == i) return cui;
	}

	return nullptr;
}

void GenericControllableContainerEditor::buttonClicked(Button * b)
{
	if (b == expandBT) setCollapsed(false);
	if (b == collapseBT) setCollapsed(true);
}

void GenericControllableContainerEditor::labelTextChanged(Label * l)
{
	if (l == &containerLabel) container->setUndoableNiceName(l->getText());
}

void GenericControllableContainerEditor::addControllableUI(Controllable * c, bool resize)
{
	if (!c->isControllableExposed || c->hideInEditor) return;

	InspectableEditor * cui = c->getEditor(false);
	int index = container->controllables.indexOf(c);
	childEditors.insert(index, cui);
	addAndMakeVisible(cui);
	if (resize) resized();
}

void GenericControllableContainerEditor::removeControllableUI(Controllable * c, bool resize)
{
	InspectableEditor * cui = getEditorForInspectable(c);
	if (cui == nullptr) return;

	removeChildComponent(cui);
	childEditors.removeObject(cui);
	if (resize) resized();

}

void GenericControllableContainerEditor::newMessage(const ContainerAsyncEvent & p)
{
	switch (p.type)
	{
	case ContainerAsyncEvent::ControllableAdded:
		//if (p.targetControllable->parentContainer != container) return;
		//if (p.targetControllable->hideInEditor) return;
		//addControllableUI(p.targetControllable, true);
		resetAndBuild();
		resized();
		break;

	case ContainerAsyncEvent::ControllableRemoved:
		removeControllableUI(p.targetControllable, true);
		//resetAndBuild();
		//resized();
		break;

	case ContainerAsyncEvent::ControllableContainerAdded:
		resetAndBuild();
		resized();
		break;

	case ContainerAsyncEvent::ControllableContainerRemoved:
		removeEditorUI(p.targetContainer, true);
		break;

	case ContainerAsyncEvent::ChildStructureChanged:
		//nothing ?
		break;

	case ContainerAsyncEvent::ControllableContainerReordered:
		resetAndBuild();
		resized();
		break;

	case ContainerAsyncEvent::ControllableFeedbackUpdate:
		controllableFeedbackUpdate(p.targetControllable);
		break;

	case ContainerAsyncEvent::ChildAddressChanged:
		containerLabel.setText(container->niceName, dontSendNotification);
		break;

	default:
		//not handled
		break;

	}
}

void GenericControllableContainerEditor::childBoundsChanged(Component * c)
{
	if (isRebuilding) return;
	if (getWidth() == 0 || getHeight() == 0) return;

	resized();

}

void GenericControllableContainerEditor::changeListenerCallback(ChangeBroadcaster * source)
{
	if (source == &collapseAnimator)
	{
		if (!collapseAnimator.isAnimating() && container->editorIsCollapsed) resetAndBuild();
	}
}


void GenericControllableContainerEditor::paint(Graphics & g)
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

	if(!isRoot && !container->hideEditorHeader)
	{
		g.setColour(contourColor.withAlpha(.4f));
		juce::Rectangle<int> r = getLocalBounds();
		if (container->editorIsCollapsed && container->editorCanBeCollapsed) r.setHeight(headerHeight);
		g.fillRoundedRectangle(r.toFloat(), 4);
	}

	if ((isRoot || canBeCollapsed()) && !container->hideEditorHeader)
	{
		g.setColour(contourColor.withAlpha(isRoot?.8f:.4f));
		g.fillRoundedRectangle(getHeaderBounds().toFloat(), 4);
	}
	
	if (!isRoot && !container->hideEditorHeader)
	{
		g.setColour(contourColor.brighter(.2f));
		g.drawRoundedRectangle(getLocalBounds().toFloat(), 4, 2);
	}
	
}

void GenericControllableContainerEditor::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	isRebuilding = true;
	resizedInternal(r);
	isRebuilding = false;
	if(!collapseAnimator.isAnimating() && !prepareToAnimate) setSize(getWidth(), (!isRoot && container->editorIsCollapsed)?headerHeight:jmax<int>(r.getY()+2, headerHeight));
}

void GenericControllableContainerEditor::resizedInternal(juce::Rectangle<int>& r)
{
	if (container == nullptr)
	{
		DBG("Container is null !");
		resetAndBuild();
		return;
	}

	if (!container->hideEditorHeader)
	{
		juce::Rectangle<int> hr = r.removeFromTop(headerHeight);

		if (canBeCollapsed()) //draw arrow here to have better control in resizedInternalHeader overrides
		{
			juce::Rectangle<int> ar = hr.removeFromLeft(headerHeight).reduced(4);
			if (container->editorIsCollapsed) expandBT->setBounds(ar);
			else collapseBT->setBounds(ar);
			r.removeFromLeft(2);
		}

		resizedInternalHeader(hr);
		r.removeFromTop(headerGap);
	}
	
	r.reduce(2, 2);
	if(!isRoot && !container->hideEditorHeader) r.removeFromLeft(4);
	if (!canBeCollapsed() || !container->editorIsCollapsed) resizedInternalContent(r);
}

void GenericControllableContainerEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	containerLabel.setBounds(r.removeFromLeft(containerLabel.getFont().getStringWidth(containerLabel.getText())+20));
	headerSpacer.setBounds(r);
}

void GenericControllableContainerEditor::resizedInternalContent(juce::Rectangle<int>& r)
{
	for (auto &cui : childEditors)
	{
		
		int th = cui->getHeight();
		cui->setBounds(r.withHeight(th));
		
		float gap = 4;
		if (isRoot && dynamic_cast<GenericControllableContainerEditor *>(cui) != nullptr) gap = 16;
		
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
	return !isRoot && container->editorCanBeCollapsed && !container->hideEditorHeader;
}



//EnablingControllableContainerEditor

EnablingControllableContainerEditor::EnablingControllableContainerEditor(EnablingControllableContainer * cc, bool isRoot, bool buildAtCreation) :
	GenericControllableContainerEditor(cc, isRoot, buildAtCreation),
	ioContainer(cc)
{
	if (cc->canBeDisabled)
	{
		enabledUI = ioContainer->enabled->createImageToggle(AssetManager::getInstance()->getPowerBT());
		addAndMakeVisible(enabledUI);
	}
}

void EnablingControllableContainerEditor::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if(((EnablingControllableContainer *)container.get())->canBeDisabled) enabledUI->setBounds(r.removeFromLeft(r.getHeight()).reduced(2));
	GenericControllableContainerEditor::resizedInternalHeader(r);
}

void EnablingControllableContainerEditor::controllableFeedbackUpdate(Controllable * c)
{
	if (c == ioContainer->enabled)
	{
		setCollapsed(!ioContainer->enabled->boolValue(), true);
	}
}
