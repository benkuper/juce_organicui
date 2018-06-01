/*
  ==============================================================================

    GenericManagerEditor.h
    Created: 19 Dec 2016 4:29:28pm
    Author:  Ben

  ==============================================================================
*/

#ifndef GENERICMANAGEREDITOR_H_INCLUDED
#define GENERICMANAGEREDITOR_H_INCLUDED

template<class T>
class BaseManager;

template<class T>
class GenericManagerEditor :
	public EnablingControllableContainerEditor,
	public BaseManager<T>::AsyncListener
{
public:
	GenericManagerEditor(BaseManager<T> * manager, bool isRoot);
	virtual ~GenericManagerEditor();

	BaseManager<T> * manager;

	String noItemText;

	//layout
	bool fixedItemHeight;
	int gap = 2;

	void resetAndBuild() override;
	void addExistingItems();

	
	//menu
	ScopedPointer<ImageButton> addItemBT;
	String addItemText;

	void paint(Graphics &g) override;
	virtual void resizedInternalHeader(juce::Rectangle<int> &r) override;

	virtual void showMenuAndAddItem(bool isFromAddButton);
	virtual T * addItemFromMenu(bool isFromAddButton);

	void buttonClicked(Button *) override;
	void mouseDown(const MouseEvent &e) override;

	void newMessage(const typename BaseManager<T>::ManagerEvent &e) override;

	virtual void itemAddedAsync(T * item) {}
	virtual void itemRemovedAsync(T * item) {}
};



template<class T>
GenericManagerEditor<T>::GenericManagerEditor(BaseManager<T> * _manager, bool isRoot) :
	EnablingControllableContainerEditor(_manager, isRoot, false),
	manager(_manager),
	addItemText("Add item")
{
	headerHeight = 20;
	setInterceptsMouseClicks(true, true);

	if (manager->userCanAddItemsManually)
	{
		addItemBT = AssetManager::getInstance()->getAddBT();
		addAndMakeVisible(addItemBT);
		addItemBT->addListener(this);
	}
	
	manager->addAsyncManagerListener(this);

	resetAndBuild();
}

template<class T>
GenericManagerEditor<T>::~GenericManagerEditor()
{
	if(!inspectable.wasObjectDeleted()) manager->removeAsyncManagerListener(this);
}

template<class T>
void GenericManagerEditor<T>::resetAndBuild()
{
	GenericControllableContainerEditor::resetAndBuild(); 
	resized();

	for (auto &e : childEditors)
	{
		BaseItemEditor * be = dynamic_cast<BaseItemEditor *>(e);
		if (be == nullptr) continue;
		
		int index = manager->items.indexOf(static_cast<T *>(be->item));
		be->setIsFirst(index == 0);
		be->setIsLast(index == manager->items.size() - 1);
	}
}

template<class T>
void GenericManagerEditor<T>::addExistingItems()
{
	resetAndBuild();
}

template<class T>
void GenericManagerEditor<T>::paint(Graphics & g)
{
	GenericControllableContainerEditor::paint(g);

	if (this->manager->items.size() == 0 && this->noItemText.isNotEmpty())
	{
		g.setColour(PANEL_COLOR.brighter(.1f));
		g.drawFittedText(this->noItemText, this->getContentBounds().reduced(10), Justification::centred, 4);
	}
}


template<class T>
void GenericManagerEditor<T>::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (addItemBT != nullptr)
	{
		addItemBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
		r.removeFromRight(2);
	}

	EnablingControllableContainerEditor::resizedInternalHeader(r);
}

template<class T>
void GenericManagerEditor<T>::showMenuAndAddItem(bool isFromAddButton)
{
	if (manager->managerFactory != nullptr)
	{
		T * m = manager->managerFactory->showCreateMenu();
		if (m != nullptr)
		{
			manager->addItem(m);
		}
	} else
	{
		if (isFromAddButton)
		{
			addItemFromMenu(true);
			return;
		}

		PopupMenu p;
		p.addItem(1, addItemText);

		int result = p.show();
		switch (result)
		{
		case 1:
			addItemFromMenu(isFromAddButton);
			break;
		}
	}

	
}

template<class T>
T * GenericManagerEditor<T>::addItemFromMenu(bool /*isFromAddButton*/)
{
	T * item = manager->BaseManager<T>::addItem();
	return item;
}

template<class T>
void GenericManagerEditor<T>::buttonClicked(Button * b)
{
	GenericControllableContainerEditor::buttonClicked(b);

	if (b == addItemBT)
	{
		showMenuAndAddItem(true);
	}
} 

template<class T>
void GenericManagerEditor<T>::mouseDown(const MouseEvent & e)
{
	GenericControllableContainerEditor::mouseDown(e);

	if (e.mods.isLeftButtonDown())
	{
	} else if (e.mods.isRightButtonDown())
	{
		showMenuAndAddItem(false);
	}
}

template<class T>
void GenericManagerEditor<T>::newMessage(const typename BaseManager<T>::ManagerEvent & e)
{
	switch (e.type)
	{
	case BaseManager<T>::ManagerEvent::ITEM_ADDED:
		setCollapsed(false, true);
		itemAddedAsync(e.getItem());
		break;

	case BaseManager<T>::ManagerEvent::ITEM_REMOVED:
		itemRemovedAsync(e.getItem());
		break;

	case BaseManager<T>::ManagerEvent::ITEMS_REORDERED:
		resetAndBuild();
		break;

	default:
		break;
	}
}

#endif  // GENERICMANAGEREDITOR_H_INCLUDED
