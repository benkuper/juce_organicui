/*
  ==============================================================================

	Outliner.cpp
	Created: 7 Oct 2016 10:31:23am
	Author:  bkupe

  ==============================================================================
*/


juce_ImplementSingleton(Outliner)

Outliner::Outliner(const String& contentName) :
	ShapeShifterContentComponent(contentName),
	hideShowState(false),
	enabled(true)
{
	if (Engine::mainEngine != nullptr)
	{
		Engine::mainEngine->addControllableContainerListener(this);
		Engine::mainEngine->addEngineListener(this);
	}

	rootItem.reset(new OutlinerItem(Engine::mainEngine, false, false));
	treeView.setRootItem(rootItem.get());
	treeView.setRootItemVisible(false);
	addAndMakeVisible(treeView);
	treeView.getViewport()->setScrollBarThickness(10);

	remoteHideShowAllBT.reset(AssetManager::getInstance()->getToggleBTImage(AssetManager::getInstance()->eyeImage));
	remoteHideShowAllBT->addListener(this);
	addAndMakeVisible(remoteHideShowAllBT.get());

	searchBar.setJustificationType(Justification::topLeft);
	searchBar.setColour(searchBar.backgroundColourId, BG_COLOR.darker(.1f).withAlpha(.7f));
	searchBar.setColour(searchBar.outlineColourId, BG_COLOR.brighter(.1f));
	searchBar.setColour(searchBar.textColourId, TEXT_COLOR.darker(.3f));
	searchBar.setFont(10);
	searchBar.setColour(CaretComponent::caretColourId, Colours::orange);
	searchBar.setEditable(true);
	searchBar.addListener(this);
	addAndMakeVisible(searchBar);

	rebuildTree();


	helpID = "Outliner";

}

Outliner::~Outliner()
{
	//DBG("Outliner destroy, engine ?" << (int)Engine::mainEngine);
	treeView.setRootItem(nullptr);
	if (Engine::mainEngine != nullptr)
	{
		Engine::mainEngine->removeControllableContainerListener(this);
		Engine::mainEngine->removeEngineListener(this);
	}

}

void Outliner::clear()
{
	rootItem->clearSubItems();
}

void Outliner::setEnabled(bool value)
{
	if (enabled == value) return;
	if (enabled)
	{
		Engine::mainEngine->addControllableContainerListener(this);
		rebuildTree();
	}
	else
	{
		clear();
		Engine::mainEngine->removeControllableContainerListener(this);
	}
}

void Outliner::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	juce::Rectangle<int> hr = r.removeFromTop(20);
	searchBar.setBounds(hr.removeFromLeft(120).reduced(1));
	remoteHideShowAllBT->setBounds(hr.removeFromRight(hr.getHeight()).reduced(2));

	treeView.setBounds(r);
}

void Outliner::paint(Graphics& g)
{
	//g.fillAll(Colours::green.withAlpha(.2f));
}




void Outliner::rebuildTree(ControllableContainer* fromContainer)
{
	if (Engine::mainEngine == nullptr) return;

	if (fromContainer == nullptr) fromContainer = Engine::mainEngine;

	updateFilteredList();

	OutlinerItem* fromItem = getItemForContainer(fromContainer);
	fromItem->clearSubItems();

	std::unique_ptr<XmlElement> os = treeView.getOpennessState(true);

	buildTree(fromItem, fromContainer, false);
	rootItem->setOpen(true);

	if (!listIsFiltered) treeView.restoreOpennessState(*os, true);
}

void Outliner::buildTree(OutlinerItem* parentItem, ControllableContainer* parentContainer, bool parentsHaveHideInRemote)
{
	if (parentContainer == nullptr) return;
	Array<WeakReference<ControllableContainer>> childContainers = parentContainer->getAllContainers(false);

	parentsHaveHideInRemote |= parentContainer->hideInRemoteControl;

	parentContainer->controllableContainers.getLock().enter();
	for (auto& cc : parentContainer->controllableContainers)
	{
		bool forceOpen = false;
		bool isFiltered = false;
		bool passed = false;
		if (listIsFiltered)
		{
			if (filteredContainers.contains(cc))
			{
				isFiltered = true;
				passed = true;
				filteredContainers.removeAllInstancesOf(cc);
			}

			if (parentsOfFiltered.contains(cc))
			{
				forceOpen = true;
				passed = true;
				parentsOfFiltered.removeAllInstancesOf(cc);
			}

			if (childOfFiltered.contains(cc))
			{
				passed = true;
				childOfFiltered.removeAllInstancesOf(cc);
			}

			if (!passed) continue; //not found
		}

		OutlinerItem* ccItem = createItem(cc, parentsHaveHideInRemote, isFiltered);
		if (forceOpen) ccItem->setOpen(true);
		parentItem->addSubItem(ccItem);
		buildTree(ccItem, cc, parentsHaveHideInRemote);
	}
	parentContainer->controllableContainers.getLock().exit();

	parentContainer->controllables.getLock().enter();
	for (auto& c : parentContainer->controllables)
	{
		bool isFiltered = false;

		if (listIsFiltered)
		{
			if (filteredControllables.contains(c))
			{
				isFiltered = true;
				filteredControllables.removeAllInstancesOf(c);
			}
			else if (childOfFiltered.contains(c))
			{
				childOfFiltered.removeAllInstancesOf(c);
			}
			else continue;
		}

		OutlinerItem* cItem = createItem(c, parentsHaveHideInRemote, isFiltered);
		parentItem->addSubItem(cItem);
	}
	parentContainer->controllables.getLock().exit();
}

void Outliner::updateFilteredList()
{
	String searchFilter = searchBar.getText().toLowerCase();

	filteredContainers.clear();
	filteredControllables.clear();
	parentsOfFiltered.clear();
	childOfFiltered.clear();

	listIsFiltered = searchFilter.isNotEmpty();
	if (listIsFiltered)
	{
		Array<WeakReference<Controllable>> cont = Engine::mainEngine->getAllControllables(true);
		Array<WeakReference<ControllableContainer>> containers = Engine::mainEngine->getAllContainers(true);

		for (auto& c : cont)
		{
			if (c->niceName.toLowerCase().contains(searchFilter))
			{
				filteredControllables.add(c);
				WeakReference<ControllableContainer> pc = c->parentContainer;
				while (pc != Engine::mainEngine && pc != nullptr)
				{
					parentsOfFiltered.addIfNotAlreadyThere(pc);
					//containers.removeAllInstancesOf(pc);
					pc = pc->parentContainer;
				}
			}
		}

		for (auto& cc : containers)
		{
			if (cc->niceName.toLowerCase().contains(searchFilter))
			{
				filteredContainers.addIfNotAlreadyThere(cc);

				Array<WeakReference<Controllable>> childControllables = cc->getAllControllables(true);
				for (auto& ccc : childControllables) childOfFiltered.addIfNotAlreadyThere(ccc);

				Array<WeakReference<ControllableContainer>> childContainers = cc->getAllContainers(true);
				for (auto& ccont : childContainers) childOfFiltered.addIfNotAlreadyThere(ccont);

				WeakReference<ControllableContainer> pc = cc->parentContainer;
				while (pc != Engine::mainEngine && pc != nullptr)
				{
					parentsOfFiltered.addIfNotAlreadyThere(pc);
					pc = pc->parentContainer;
				}
			}
		}
	}
}

OutlinerItem* Outliner::createItem(WeakReference<ControllableContainer> container, bool parentsHaveHideInRemote, bool isFiltered)
{
	return new OutlinerItem(container, parentsHaveHideInRemote, isFiltered);
}

OutlinerItem* Outliner::createItem(WeakReference<Controllable> controllable, bool parentsHaveHideInRemote, bool isFiltered)
{
	return new OutlinerItem(controllable, parentsHaveHideInRemote, isFiltered);
}

void Outliner::fileLoaded()
{
	MessageManagerLock mmLock;
	rebuildTree();
}

void Outliner::engineCleared()
{
	MessageManagerLock mmLock;
	rebuildTree();
}

OutlinerItem* Outliner::getItemForContainer(ControllableContainer* cc)
{
	if (cc == nullptr || cc == Engine::mainEngine) return rootItem.get();

	Array<int> containerIndices;
	ControllableContainer* cParent = cc;
	while (cParent != Engine::mainEngine && cParent != nullptr)
	{
		ControllableContainer* parent = cParent->parentContainer;
		if (parent == nullptr) return nullptr;
		containerIndices.add(parent->controllableContainers.indexOf(cParent));
	}

	OutlinerItem* result = rootItem.get();
	for (auto& index : containerIndices)
	{
		jassert(result != nullptr);
		result = (OutlinerItem*)result->getSubItem(index);
	}

	return result;
}

void Outliner::childStructureChanged(ControllableContainer*)
{
	if (enabled && !Engine::mainEngine->isClearing && !Engine::mainEngine->isLoadingFile)
	{
		MessageManagerLock mmLock;
		rebuildTree();
	}

}

void Outliner::labelTextChanged(Label* label)
{
	if (label == &searchBar)
	{
		rebuildTree();
	}
}

void Outliner::buttonClicked(Button* b)
{
	if (b == remoteHideShowAllBT.get())
	{
		Array<WeakReference<Controllable>> cont = Engine::mainEngine->getAllControllables(true);
		Array<WeakReference<ControllableContainer>> containers = Engine::mainEngine->getAllContainers(true);

		for (auto& c : cont) c->hideInRemoteControl = hideShowState ? true : c->defaultHideInRemoteControl;
		for (auto& cc : containers) cc->hideInRemoteControl = hideShowState ? true : cc->defaultHideInRemoteControl;

		hideShowState = !hideShowState;
		rebuildTree();
	}
}


// OUTLINER ITEM

OutlinerItem::OutlinerItem(WeakReference<ControllableContainer> _container, bool parentsHaveHideInRemote, bool isFiltered) :
	InspectableContent(_container),
	isContainer(true),
	itemName(_container->niceName),
	parentsHaveHideInRemote(parentsHaveHideInRemote),
	isFiltered(isFiltered),
	container(_container),
	controllable(nullptr)
{
	container->addControllableContainerListener(this);
}

OutlinerItem::OutlinerItem(WeakReference<Controllable> _controllable, bool parentsHaveHideInRemote, bool isFiltered) :
	InspectableContent(_controllable),
	isContainer(false),
	itemName(_controllable->niceName),
	parentsHaveHideInRemote(parentsHaveHideInRemote),
	isFiltered(isFiltered),
	container(nullptr),
	controllable(_controllable)
{
}

OutlinerItem::~OutlinerItem()
{
	if (isContainer && !container.wasObjectDeleted()) container->removeControllableContainerListener(this);
	masterReference.clear();
}


bool OutlinerItem::mightContainSubItems()
{
	return isContainer;
}

std::unique_ptr<Component> OutlinerItem::createItemComponent()
{
	return std::unique_ptr<Component>(new OutlinerItemComponent(this));
}

String OutlinerItem::getUniqueName() const
{
	if ((isContainer && container.wasObjectDeleted()) || (!isContainer && controllable.wasObjectDeleted())) return "";

	String n = isContainer ? container.get()->getControlAddress() : controllable.get()->getControlAddress();
	if (n.isEmpty()) n = isContainer ? container->shortName : controllable->shortName;

	return n;
}

void OutlinerItem::childAddressChanged(ControllableContainer*)
{
	if (isContainer)
	{
		itemName = container->niceName;
		itemListeners.call(&OutlinerItemListener::itemNameChanged);
	}
}


void OutlinerItem::inspectableSelectionChanged(Inspectable* i)
{
	InspectableContent::inspectableSelectionChanged(i);

	MessageManagerLock mmLock;
	if (!mmLock.lockWasGained()) return;

	setSelected(inspectable->isSelected, true);

	if (inspectable->isSelected)
	{
		//close everything before ?

		//open all parents to view the item
		if (!areAllParentsOpen())
		{
			TreeViewItem* ti = getParentItem();
			while (ti != nullptr)
			{
				if (!ti->isOpen()) ti->setOpen(true);
				ti = ti->getParentItem();
			}
		}
	}

}

void OutlinerItem::setHideInRemote(bool value)
{
	for (int i = 0; i < getNumSubItems(); i++)
	{
		((OutlinerItem*)getSubItem(i))->setParentsHaveHideInRemote(inspectable->hideInRemoteControl || parentsHaveHideInRemote);
	}
}

void OutlinerItem::setParentsHaveHideInRemote(bool value)
{
	if (parentsHaveHideInRemote == value) return;
	parentsHaveHideInRemote = value;

	for (int i = 0; i < getNumSubItems(); i++)
	{
		((OutlinerItem*)getSubItem(i))->setParentsHaveHideInRemote(inspectable->hideInRemoteControl || parentsHaveHideInRemote);
	}

	itemListeners.call(&OutlinerItemListener::hideRemoteChanged);
}


// OutlinerItemComponent

OutlinerItemComponent::OutlinerItemComponent(OutlinerItem* _item) :
	InspectableContentComponent(_item->inspectable),
	item(_item),
	label("label", _item->itemName)
{
	if (item->isContainer && item->container == nullptr)
	{
		jassertfalse;
		return;
	}

	item->addItemListener(this);
	autoDrawContourWhenSelected = false;
	setTooltip(item->isContainer ? item->container->getControlAddress() : item->controllable->description + "\nControl Address : " + item->controllable->controlAddress);
	addAndMakeVisible(&label);

	if (item->isContainer) color = item->container->nameCanBeChangedByUser ? HIGHLIGHT_COLOR : TEXT_COLOR;
	else color = BLUE_COLOR.brighter(item->controllable->type == Controllable::TRIGGER);

	label.setFont(label.getFont().withHeight(12));
	label.setColour(label.backgroundWhenEditingColourId, Colours::white);
	label.setColour(Label::textColourId, inspectable->isSelected ? Colours::grey.darker() : color);


	if (item->isContainer && item->container->nameCanBeChangedByUser)
	{
		label.addListener(this);
		label.setEditable(false, true);
		label.addMouseListener(this, true);
	}
	else
	{
		label.setInterceptsMouseClicks(false, false);
	}

	hideInRemoteBT.reset(AssetManager::getInstance()->getToggleBTImage(AssetManager::getInstance()->eyeImage));
	hideInRemoteBT->setToggleState(!item->inspectable->hideInRemoteControl, dontSendNotification);
	hideInRemoteBT->setAlpha(item->parentsHaveHideInRemote ? .5f : 1);

	hideInRemoteBT->setTooltip("Hide in Remote Control. If remote control is enabled in Preferences, this decides whether to expose or not this (and all its children) to the OSCQuery structure");

	hideInRemoteBT->addListener(this);

	addAndMakeVisible(hideInRemoteBT.get());

	resized();
}

OutlinerItemComponent::~OutlinerItemComponent()
{
	if (!item.wasObjectDeleted()) item->removeItemListener(this);
}

void OutlinerItemComponent::paint(Graphics& g)
{
	if (inspectable == nullptr || inspectable.wasObjectDeleted()) return;

	juce::Rectangle<int> r = getLocalBounds();

	int labelWidth = label.getFont().getStringWidth(label.getText());

	juce::Rectangle<float> lr = r.withSize(labelWidth + 20, r.getHeight()).toFloat();
	if (inspectable->isSelected)
	{
		g.setColour(color);
		g.fillRoundedRectangle(lr, 2);
	}

	if (item->isFiltered)
	{
		g.setColour(YELLOW_COLOR.withAlpha(.3f));
		g.fillRoundedRectangle(lr.reduced(1), 2);

		g.setColour(YELLOW_COLOR.withAlpha(.6f));
		g.drawRoundedRectangle(lr.reduced(1), 2, 1);
	}
}

void OutlinerItemComponent::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	if (r.isEmpty()) return;
	if (hideInRemoteBT == nullptr) return;
	hideInRemoteBT->setBounds(r.removeFromRight(r.getHeight()).reduced(1));
	r.removeFromLeft(3);
	resizedInternal(r);
	label.setBounds(r);
}

void OutlinerItemComponent::buttonClicked(Button* b)
{
	if (b == hideInRemoteBT.get())
	{
		item->inspectable->hideInRemoteControl = !item->inspectable->hideInRemoteControl;
		hideInRemoteBT->setToggleState(!item->inspectable->hideInRemoteControl, dontSendNotification);
		item->setHideInRemote(item->inspectable->hideInRemoteControl);
	}
}

void OutlinerItemComponent::labelTextChanged(Label*)
{
	if (item.wasObjectDeleted()) return;
	item->container->setUndoableNiceName(label.getText());
}

void OutlinerItemComponent::mouseDown(const MouseEvent& e)
{
	InspectableContentComponent::mouseDown(e);

	if (e.mods.isRightButtonDown())
	{
		PopupMenu p;
		p.addItem(-1, "Copy OSC Control Address");
		p.addItem(-2, "Copy Script Control Address");

		p.addSeparator();

		if (item->controllable != nullptr)
		{
			Controllable::Type cType = item->controllable->type;
			if (cType == Controllable::FLOAT ||
				cType == Controllable::INT ||
				cType == Controllable::POINT2D ||
				cType == Controllable::POINT3D ||
				cType == Controllable::COLOR ||
				cType == Controllable::BOOL)
			{
				p.addItem(-10, "Watch this with The Detective");
			}
		}

		PopupMenu dashboardMenu;
		int index = 0;
		for (auto& di : DashboardManager::getInstance()->items)
		{
			dashboardMenu.addItem(index + 10000, di->niceName);
			index++;
		}
		p.addSubMenu("Send to Dashboard", dashboardMenu);

		p.showMenuAsync(PopupMenu::Options(), [this](int result)
			{
				OutlinerItem* item = this->item;

				switch (result)
				{
				case -1:
					if (item->isContainer) SystemClipboard::copyTextToClipboard(item->container->getControlAddress());
					else SystemClipboard::copyTextToClipboard(item->controllable->controlAddress);
					break;
				case -2:
					if (item->isContainer)  SystemClipboard::copyTextToClipboard("root" + item->container->getControlAddress().replaceCharacter('/', '.'));
					else SystemClipboard::copyTextToClipboard("root" + item->controllable->controlAddress.replaceCharacter('/', '.'));
					break;

				case -10:
					Detective::getInstance()->watchControllable(item->controllable);
					break;

				default:
					if (result >= 10000)
					{
						DashboardManager::getInstance()->items[result - 10000]->itemManager.addItem((item->isContainer ? item->container->createDashboardItem() : item->controllable->createDashboardItem()));
					}
					break;

				}
			}
		);
	}
}

void OutlinerItemComponent::inspectableSelectionChanged(Inspectable* i)
{
	label.setColour(Label::textColourId, inspectable->isSelected ? Colours::grey.darker() : color);
	repaint();
}

void OutlinerItemComponent::itemNameChanged()
{
	label.setText(item->container->niceName, dontSendNotification);
}

void OutlinerItemComponent::hideRemoteChanged()
{
	hideInRemoteBT->setAlpha(item->parentsHaveHideInRemote ? .5f : 1);
}

