#include "Outliner.h"
/*
  ==============================================================================

	Outliner.cpp
	Created: 7 Oct 2016 10:31:23am
	Author:  bkupe

  ==============================================================================
*/


juce_ImplementSingleton(Outliner)

Outliner::Outliner(const String &contentName) :
	ShapeShifterContentComponent(contentName),
	enabled(true)
{

	if (Engine::mainEngine != nullptr) Engine::mainEngine->addControllableContainerListener(this);

	showHiddenContainers = false;

	rootItem.reset(new OutlinerItem(Engine::mainEngine, false));
	treeView.setRootItem(rootItem.get());
	treeView.setRootItemVisible(false);
	addAndMakeVisible(treeView);
	treeView.getViewport()->setScrollBarThickness(10);

	rebuildTree();

	helpID = "Outliner";

}

Outliner::~Outliner()
{
	//DBG("Outliner destroy, engine ?" << (int)Engine::mainEngine);
	treeView.setRootItem(nullptr);
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeControllableContainerListener(this);

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
	r.removeFromTop(20);
	treeView.setBounds(r);
}

void Outliner::paint(Graphics & g)
{
	//g.fillAll(Colours::green.withAlpha(.2f));
}


void Outliner::rebuildTree(ControllableContainer* fromContainer)
{
	if (Engine::mainEngine == nullptr) return;

	if (fromContainer == nullptr) fromContainer = Engine::mainEngine;
	OutlinerItem* fromItem = getItemForContainer(fromContainer);
	fromItem->clearSubItems();

	std::unique_ptr<XmlElement> os = treeView.getOpennessState(true);
	buildTree(fromItem, fromContainer, false);
	rootItem->setOpen(true);

	treeView.restoreOpennessState(*os, true);
}

void Outliner::buildTree(OutlinerItem * parentItem, ControllableContainer * parentContainer, bool parentsHaveHideInRemote)
{
	if (parentContainer == nullptr) return;
	Array<WeakReference<ControllableContainer>> childContainers = parentContainer->getAllContainers(false);
	
	parentsHaveHideInRemote |= parentContainer->hideInRemoteControl;

	parentContainer->controllableContainers.getLock().enter();
	for (auto &cc : parentContainer->controllableContainers)
	{
		/*if (cc->skipControllableNameInAddress && !showHiddenContainers)
		{
			buildTree(parentItem, cc);
		}
		else
		{*/

			OutlinerItem * ccItem = new OutlinerItem(cc, parentsHaveHideInRemote);
			parentItem->addSubItem(ccItem);

			buildTree(ccItem, cc, parentsHaveHideInRemote);
		//}
	}
	parentContainer->controllableContainers.getLock().exit();

	parentContainer->controllables.getLock().enter();
	for (auto &c : parentContainer->controllables)
	{
		OutlinerItem * cItem = new OutlinerItem(c, parentsHaveHideInRemote);
		parentItem->addSubItem(cItem);
	}
	parentContainer->controllables.getLock().exit();

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
		result = (OutlinerItem *)result->getSubItem(index);
	}

	return result;
}

void Outliner::childStructureChanged(ControllableContainer *)
{
	if (enabled)
	{
		MessageManagerLock mmLock;
		rebuildTree();
	}

}


// OUTLINER ITEM

OutlinerItem::OutlinerItem(WeakReference<ControllableContainer> _container, bool parentsHaveHideInRemote) :
	InspectableContent(_container),
	isContainer(true),
	itemName(_container->niceName),
	container(_container),
	controllable(nullptr),
	parentsHaveHideInRemote(parentsHaveHideInRemote)
{
	container->addControllableContainerListener(this);
}

OutlinerItem::OutlinerItem(WeakReference<Controllable> _controllable, bool parentsHaveHideInRemote) :
	InspectableContent(_controllable),
	isContainer(false),
	itemName(_controllable->niceName),
	container(nullptr),
	controllable(_controllable),
	parentsHaveHideInRemote(parentsHaveHideInRemote)
{
}

OutlinerItem::~OutlinerItem()
{
	if (isContainer) container->removeControllableContainerListener(this);
	masterReference.clear();
}


bool OutlinerItem::mightContainSubItems()
{
	return isContainer;
}

Component * OutlinerItem::createItemComponent()
{
	return new OutlinerItemComponent(this);
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


void OutlinerItem::inspectableSelectionChanged(Inspectable * i)
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
			TreeViewItem * ti = getParentItem();
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

OutlinerItemComponent::OutlinerItemComponent(OutlinerItem * _item) :
	InspectableContentComponent(_item->inspectable),
	item(_item),
	label("label", _item->itemName)
{
	item->addItemListener(this);
	autoDrawContourWhenSelected = false;
	setTooltip(item->isContainer ? item->container->getControlAddress() : item->controllable->description + "\nControl Address : " + item->controllable->controlAddress);
	addAndMakeVisible(&label);

	color = BLUE_COLOR;
	if (item->isContainer) color = item->container->nameCanBeChangedByUser ? HIGHLIGHT_COLOR : TEXT_COLOR;


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

void OutlinerItemComponent::paint(Graphics & g)
{
	if (inspectable.wasObjectDeleted()) return;

	juce::Rectangle<int> r = getLocalBounds();
	
	int labelWidth = label.getFont().getStringWidth(label.getText());

	if (inspectable->isSelected)
	{
		g.setColour(color);
		g.fillRoundedRectangle(r.withSize(labelWidth + 20, r.getHeight()).toFloat(), 2);
	}

}

void OutlinerItemComponent::resized()
{
	Rectangle<int> r = getLocalBounds();
	if (r.isEmpty()) return;
	hideInRemoteBT->setBounds(r.removeFromRight(r.getHeight()).reduced(1));
	r.removeFromLeft(3);
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

void OutlinerItemComponent::labelTextChanged(Label *)
{
	if (item.wasObjectDeleted()) return;
	item->container->setUndoableNiceName(label.getText());
}

void OutlinerItemComponent::mouseDown(const MouseEvent &e)
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
				cType == Controllable::BOOL )
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
		
		int result = p.show();
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
				DashboardManager::getInstance()->items[result - 10000]->itemManager.addItem((item->isContainer?item->container->createDashboardItem():item->controllable->createDashboardItem()));
			}
			break;

		}
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

