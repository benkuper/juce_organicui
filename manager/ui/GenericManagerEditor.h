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
	public BaseManager<T>::Listener
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
};



template<class T>
GenericManagerEditor<T>::GenericManagerEditor(BaseManager<T> * _manager, bool isRoot) :
	EnablingControllableContainerEditor(_manager, isRoot),
	manager(_manager),
	addItemText("Add item")
{
	headerHeight = 20;
	setInterceptsMouseClicks(true, true);

	addItemBT = AssetManager::getInstance()->getAddBT();
	addAndMakeVisible(addItemBT);
	addItemBT->addListener(this);

	manager->addBaseManagerListener(this);
}

template<class T>
GenericManagerEditor<T>::~GenericManagerEditor()
{
	manager->removeBaseManagerListener(this);
}

template<class T>
void GenericManagerEditor<T>::resetAndBuild()
{
	GenericControllableContainerEditor::resetAndBuild(); 
	resized();
}

template<class T>
void GenericManagerEditor<T>::addExistingItems()
{

	//add existing items
	for (auto &t : manager->items) addEditorUI(t, false);
	resized();
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
	addItemBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
	r.removeFromRight(2);
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
			setCollapsed(false, true);
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
	setCollapsed(false,true);
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

#endif  // GENERICMANAGEREDITOR_H_INCLUDED
