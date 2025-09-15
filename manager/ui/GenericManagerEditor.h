/*
  ==============================================================================

	GenericManagerEditor.h
	Created: 19 Dec 2016 4:29:28pm
	Author:  Ben

  ==============================================================================
*/

#pragma once


template<class T>
class GenericManagerEditor :
	public EnablingControllableContainerEditor,
	public Manager<T>::AsyncListener,
	public juce::DragAndDropTarget
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	GenericManagerEditor(Manager<T>* manager, bool isRoot);
	virtual ~GenericManagerEditor();

	Manager<T>* manager;

	juce::Array<BaseItemEditor*> itemEditors;

	juce::String noItemText;

	//drag and drop
	juce::StringArray acceptedDropTypes;
	bool isDraggingOver;
	bool highlightOnDragOver;
	int currentDropIndex;

	//layout
	bool fixedItemHeight;
	int gap = 2;

	void resetAndBuild() override;
	void addExistingItems();


	//menu
	std::unique_ptr<juce::ImageButton> addItemBT;
	juce::String addItemText;

	void paint(juce::Graphics& g) override;
	virtual void resizedInternalHeader(juce::Rectangle<int>& r) override;

	virtual void addPopupMenuItems(juce::PopupMenu* p) override;
	virtual void handleMenuSelectedID(int id) override;

	virtual void showMenuAndAddItem(bool isFromAddButton);
	virtual BaseItem* addItemFromMenu(bool isFromAddButton);

	void buttonClicked(juce::Button*) override;

	virtual bool shouldShowContainer(ControllableContainer* cc) override;

	virtual InspectableEditor* addEditorUI(ControllableContainer* cc, bool resize = false) override;
	virtual void removeEditorUI(InspectableEditor* i, bool resize = false) override;

	void newMessage(const typename ManagerEvent<T>& e) override;

	virtual void itemAddedAsync(BaseItem* item) {}
	virtual void itemRemovedAsync(BaseItem* item) {}

	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragMove(const SourceDetails& dragSourceDetails) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails& dragSourceDetails) override;
	virtual int getDropIndexForPosition(juce::Point<int> localPosition);

};



template<class T>
GenericManagerEditor<T>::GenericManagerEditor(Manager<T>* _manager, bool isRoot) :
	EnablingControllableContainerEditor(_manager, isRoot, false),
	manager(_manager),
	isDraggingOver(false),
	highlightOnDragOver(true),
	addItemText("Add item")
{
	headerHeight = 20;
	setInterceptsMouseClicks(true, true);

	if (manager->userCanAddItemsManually)
	{
		addItemBT.reset(AssetManager::getInstance()->getAddBT());
		addItemBT->setWantsKeyboardFocus(false);
		addAndMakeVisible(addItemBT.get());
		addItemBT->addListener(this);
	}

	manager->addAsyncManagerListener(this, false);

	resetAndBuild();
}

template<class T>
GenericManagerEditor<T>::~GenericManagerEditor()
{
	if (!inspectable.wasObjectDeleted()) manager->removeAsyncManagerListener(this);
}

template<class T>
void GenericManagerEditor<T>::resetAndBuild()
{
	GenericControllableContainerEditor::resetAndBuild();
	resized();

	itemEditors.clear();


	for (auto& e : childEditors)
	{
		if (e == nullptr)
		{
			DBG("Item editor is null ???");
			continue;
		}

		BaseItemEditor* be = dynamic_cast<BaseItemEditor*>(e);
		if (be == nullptr) continue;

		itemEditors.add(be);

		//int index = manager->items.indexOf(static_cast<T*>(be->item));
		//be->setIsFirst(index == 0);
		//be->setIsLast(index == manager->items.size() - 1);
	}
}

template<class T>
void GenericManagerEditor<T>::addExistingItems()
{
	resetAndBuild();
}

template<class T>
void GenericManagerEditor<T>::paint(juce::Graphics& g)
{
	GenericControllableContainerEditor::paint(g);

	if (this->manager->hasNoItems() == 0 && this->noItemText.isNotEmpty())
	{
		g.setColour(PANEL_COLOR.brighter(.1f));
		g.drawFittedText(this->noItemText, this->getContentBounds().reduced(10), juce::Justification::centred, 4);
	}

	if (isDraggingOver && highlightOnDragOver)
	{
		g.setColour(BLUE_COLOR);

		if (itemEditors.size() > 0)
		{
			BaseItemEditor* bui = itemEditors[currentDropIndex >= 0 ? currentDropIndex : itemEditors.size() - 1];
			if (bui != nullptr)
			{
				juce::Rectangle<int> buiBounds = getLocalArea(bui, bui->getLocalBounds());

				int ty = currentDropIndex >= 0 ? buiBounds.getY() - 1 : buiBounds.getBottom() + 1;
				g.drawLine(getWidth() * .25f, ty, getWidth() * .75f, ty, 2);
			}
		}
		else
		{
			int ty = this->getContentBounds().getY();
			g.drawLine(getWidth() * .25f, ty, getWidth() * .75f, ty, 2);
		}
	}
}


template<class T>
void GenericManagerEditor<T>::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (addItemBT != nullptr)
	{
		addItemBT->setBounds(r.removeFromRight(r.getHeight()));
		r.removeFromRight(2);
	}

	EnablingControllableContainerEditor::resizedInternalHeader(r);
}

template<class T>
void GenericManagerEditor<T>::addPopupMenuItems(juce::PopupMenu* p)
{
	p->addItem(1001, "Paste (add)");

	if (manager->managerFactory != nullptr) p->addSubMenu("Add...", manager->managerFactory->getMenu(1101));
	else p->addItem(1101, addItemText);

}

template<class T>
void GenericManagerEditor<T>::handleMenuSelectedID(int id)
{
	if (id > 1000 && id < 2000)
	{
		switch (id)
		{
		case 1001:
			manager->addItemsFromClipboard();
			break;

		default:
		{
			if (manager->managerFactory != nullptr)
			{
				BaseItem* item = manager->managerFactory->createFromMenuResult(id - 1100);
				if (item != nullptr) manager->addItem(item);
			}
			else
			{
				if (id == 1100) addItemFromMenu(true);
			}
		}
		break;
		}
	}
}

template<class T>
void GenericManagerEditor<T>::showMenuAndAddItem(bool isFromAddButton)
{
	if (manager->managerFactory != nullptr)
	{
		manager->managerFactory->showCreateMenu([this](BaseItem* item)
			{
				if (item != nullptr)
				{
					this->manager->addItem(item);
				}
			}
		);
	}
	else
	{
		if (isFromAddButton)
		{
			addItemFromMenu(true);
			return;
		}

		juce::PopupMenu p;
		p.addItem(1, addItemText);

		p.showMenuAsync(juce::PopupMenu::Options(), [this, isFromAddButton](int result)
			{
				switch (result)
				{
				case 1:
					this->addItemFromMenu(isFromAddButton);
					break;
				}
			}
		);
	}


}

template<class T>
BaseItem* GenericManagerEditor<T>::addItemFromMenu(bool /*isFromAddButton*/)
{
	BaseItem* item = manager->Manager<T>::addItem();
	return item;
}

template<class T>
void GenericManagerEditor<T>::buttonClicked(juce::Button* b)
{
	GenericControllableContainerEditor::buttonClicked(b);

	if (b == addItemBT.get())
	{
		showMenuAndAddItem(true);
	}
}

template<class T>
bool GenericManagerEditor<T>::shouldShowContainer(ControllableContainer* cc)
{
	if (!manager->showItemsInEditor)
	{
		if (dynamic_cast<T*>(cc) != nullptr) return false;
	}
	return GenericControllableContainerEditor::shouldShowContainer(cc);
}

template<class T>
InspectableEditor* GenericManagerEditor<T>::addEditorUI(ControllableContainer* cc, bool resize)
{
	InspectableEditor* ui = EnablingControllableContainerEditor::addEditorUI(cc, resize);
	if (BaseItemEditor* bui = dynamic_cast<BaseItemEditor*>(ui)) itemEditors.addIfNotAlreadyThere(bui);
	return ui;
}

template<class T>
void GenericManagerEditor<T>::removeEditorUI(InspectableEditor* ui, bool resize)
{
	if (BaseItemEditor* bui = dynamic_cast<BaseItemEditor*>(ui)) itemEditors.removeAllInstancesOf(bui);
	EnablingControllableContainerEditor::removeEditorUI(ui, resize);
}


template<class T>
void GenericManagerEditor<T>::newMessage(const ManagerEvent<T>& e)
{
	switch (e.type)
	{
	case ManagerEvent<T>::ITEM_ADDED:
		setCollapsed(false, true);
		itemAddedAsync(e.getItem());
		if (BaseItemEditor* bui = dynamic_cast<BaseItemEditor*>(getEditorForInspectable(e.getItem()))) itemEditors.addIfNotAlreadyThere(bui);
		resized();
		break;

	case ManagerEvent<T>::ITEM_REMOVED:
		itemRemovedAsync(e.getItem());
		if (BaseItemEditor* bui = dynamic_cast<BaseItemEditor*>(getEditorForInspectable(e.getItem()))) itemEditors.removeAllInstancesOf(bui);
		resized();
		break;

	case ManagerEvent<T>::ITEMS_ADDED:
	case ManagerEvent<T>::ITEMS_REMOVED:
	case ManagerEvent<T>::ITEMS_REORDERED:
		resetAndBuild();
		break;

	default:
		break;
	}
}


template<class T>
bool GenericManagerEditor<T>::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	juce::String dataType = dragSourceDetails.description.getProperty("dataType", "").toString();
	juce::String type = dragSourceDetails.description.getProperty("type", "").toString();
	if (acceptedDropTypes.contains(dataType) || acceptedDropTypes.contains(type)) return true;

	BaseItemEditor* itemUI = dynamic_cast<BaseItemEditor*>(dragSourceDetails.sourceComponent.get());
	if (itemEditors.contains(itemUI)) return true;

	return false;
}


template<class T>
void GenericManagerEditor<T>::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	repaint();
}


template<class T>
void GenericManagerEditor<T>::itemDragMove(const SourceDetails& dragSourceDetails)
{
	currentDropIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
	repaint();
}



template<class T>
void GenericManagerEditor<T>::itemDragExit(const SourceDetails&)
{
	isDraggingOver = false;
	repaint();
}


template<class T>
void GenericManagerEditor<T>::itemDropped(const SourceDetails& dragSourceDetails)
{
	if (BaseItemEditor* bui = dynamic_cast<BaseItemEditor*>(dragSourceDetails.sourceComponent.get()))
	{
		if (BaseItem* item = bui->item)
		{
			T* tItem = static_cast<T*>(item);
			int droppingIndex = getDropIndexForPosition(dragSourceDetails.localPosition);

			if (itemEditors.contains(bui))
			{
				if (itemEditors.indexOf(bui) < droppingIndex) droppingIndex--;
				if (droppingIndex == -1) droppingIndex = itemEditors.size() - 1;
				this->manager->setItemIndex(tItem, droppingIndex);
			}
			else
			{
				juce::var data = item->getJSONData();
				if (droppingIndex != -1) data.getDynamicObject()->setProperty("index", droppingIndex);

				if (BaseItem* newItem = this->manager->createItemFromData(data))
				{
					juce::Array<juce::UndoableAction*> actions;
					actions.add(this->manager->getAddItemUndoableAction(newItem, data));
					if (Manager<T>* sourceManager = dynamic_cast<Manager<T> *>(tItem->parentContainer.get()))
					{
						actions.addArray(sourceManager->getRemoveItemUndoableAction(tItem));
					}

					UndoMaster::getInstance()->performActions("Move " + item->niceName, actions);
				}
			}
		}
	}

	this->isDraggingOver = false;
	repaint();
}



template<class T>
int GenericManagerEditor<T>::getDropIndexForPosition(juce::Point<int> localPosition)
{
	for (int i = 0; i < itemEditors.size(); ++i)
	{
		BaseItemEditor* iui = itemEditors[i];
		juce::Point<int> p = getLocalArea(iui, iui->getLocalBounds()).getCentre();
		if (localPosition.y < p.y) return i;
	}

	return -1;
}
