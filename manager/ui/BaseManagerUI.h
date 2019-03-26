/*
  ==============================================================================

	BaseManagerUI.h
	Created: 28 Oct 2016 8:03:45pm
	Author:  bkupe

  ==============================================================================
*/

#ifndef BASEMANAGERUI_H_INCLUDED
#define BASEMANAGERUI_H_INCLUDED


#pragma warning(disable:4244)


template<class IT>
class BaseManagerItemComparator
{
public:
	BaseManagerItemComparator(BaseManager<IT> * _manager) :manager(_manager) {}

	BaseManager<IT> * manager;

	int compareElements(BaseItemMinimalUI<IT> * u1, BaseItemMinimalUI<IT> * u2)
	{
		return (manager->items.indexOf(u1->item) < manager->items.indexOf(u2->item)) ? -1 : 1;
	}
};


template<class M, class T, class U>
class BaseManagerUI;

template<class M, class T, class U>
class ManagerUIItemContainer :
	public Component
{
public:
	ManagerUIItemContainer<M, T, U>(BaseManagerUI<M, T, U> * _mui) : mui(_mui) {};
	~ManagerUIItemContainer() {}

	BaseManagerUI<M, T, U> * mui;

	void childBoundsChanged(Component * c) { mui->childBoundsChanged(c); }
};



template<class M, class T, class U>
class BaseManagerUI :
	public InspectableContentComponent,
	public BaseManager<T>::Listener,
	public BaseManager<T>::AsyncListener,
	public Button::Listener,
	public EngineListener,
	public BaseItemUI<T>::ItemUIListener,
	public ComponentListener
{
public:
	BaseManagerUI<M, T, U>(const String &contentName, M * _manager, bool _useViewport = true);
	virtual ~BaseManagerUI();

	enum Layout { HORIZONTAL, VERTICAL };

	M * manager;
	OwnedArray<U> itemsUI;
	BaseManagerItemComparator<T> managerComparator;

	//ui
	bool useViewport; //TODO, create a BaseManagerViewportUI

	Layout defaultLayout;

	ManagerUIItemContainer<M, T, U> container;
	Viewport viewport;

	Colour bgColor;
	int labelHeight;
	int minHeight;
	String managerUIName;
	bool drawContour;
	bool transparentBG;
	bool resizeOnChildBoundsChanged;
	bool autoFilterHitTestOnItems;
	bool validateHitTestOnNoItem;

	ScopedPointer<ImageButton> addItemBT;


	//ui
	String noItemText;

	//menu
	String addItemText;

	//Animation
	bool animateItemOnAdd;
	ComponentAnimator itemAnimator;

	//interaction
	BaseItemUI<T> * grabbingItem;
	int grabbingItemDropIndex;
	juce::Rectangle<int> grabSpaceRect;

	//layout
	bool fixedItemHeight;
	int gap;

	void setDefaultLayout(Layout l);

	void addExistingItems(bool resizeAfter = true);

	void setShowAddButton(bool value);


	virtual void paint(Graphics &g) override;

	virtual void resized() override;
	virtual void resizedInternalHeader(juce::Rectangle<int> &r);
	virtual void resizedInternalContent(juce::Rectangle<int> &r);
	virtual void resizedInternalFooter(juce::Rectangle<int> &r);

	virtual void updateItemsVisibility();

	virtual void childBoundsChanged(Component *) override;

	virtual bool hitTest(int x, int y) override;

	//For container check
    virtual void componentMovedOrResized(Component &c, bool wasMoved, bool wasResized) override;


	virtual void showMenuAndAddItem(bool isFromAddButton, Point<int> mouseDownPos);
	virtual void addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos);
	virtual U * addItemUI(T * item, bool animate = false, bool resizeAndRepaint = true);
	virtual U * createUIForItem(T * item);
	virtual void addItemUIInternal(U *) {}

	virtual void mouseDown(const MouseEvent &e) override;
	virtual bool keyPressed(const KeyPress &e) override;

	virtual void removeItemUI(T * item, bool resizeAndRepaint = true);
	virtual void removeItemUIInternal(U *) {}

	virtual void itemUIGrabStart(BaseItemUI<T> * se) override;
	virtual void itemUIGrabbed(BaseItemUI<T> * se) override;
	virtual void itemUIGrabEnd(BaseItemUI<T> * se) override;
	virtual void itemUIMiniModeChanged(BaseItemUI<T> * se) override {}

	//menu
	U * getUIForItem(T * item, bool directIndexAccess = true);

	int getContentHeight();

	virtual void itemAddedAsync(T * item);
	virtual void itemsAddedAsync(Array<T *> items);

	virtual void itemRemoved(T * item) override; //must keep this one realtime because the async may cause the target item to already be deleted by the time this function is called
	virtual void itemsRemoved(Array<T *> items) override;
	virtual void itemsReorderedAsync();

	void newMessage(const typename BaseManager<T>::ManagerEvent &e) override;

	void buttonClicked(Button *) override;

	virtual void inspectableDestroyed(Inspectable *) override;

	//From Engine Listener
	bool tmpAnimate;
	virtual void startLoadFile() override;
	virtual void endLoadFile() override;

	class  ManagerUIListener
	{
	public:
		/** Destructor. */
		virtual ~ManagerUIListener() {}
		virtual void itemUIAdded(U *) {}
		virtual void itemUIRemoved(U *) {}
	};

	ListenerList<ManagerUIListener> managerUIListeners;
	void addManagerUIListener(ManagerUIListener* newListener) { managerUIListeners.add(newListener); }
	void removeManagerUIListener(ManagerUIListener* listener) { managerUIListeners.remove(listener); }
};


template<class M, class T, class U>
BaseManagerUI<M, T, U>::BaseManagerUI(const String & contentName, M * _manager, bool _useViewport) :
	InspectableContentComponent(_manager),
	manager(_manager),
	managerComparator(_manager),
	useViewport(_useViewport),
	defaultLayout(VERTICAL),
	container(this),
	bgColor(BG_COLOR),
	labelHeight(10),
	minHeight(50),
	managerUIName(contentName),
	drawContour(false),
	transparentBG(false),
	resizeOnChildBoundsChanged(true),
	autoFilterHitTestOnItems(false),
	validateHitTestOnNoItem(true),
	animateItemOnAdd(true),
	grabbingItem(nullptr),
	fixedItemHeight(true),
	gap(2)
{


	//setWantsKeyboardFocus(true);

	selectionContourColor = LIGHTCONTOUR_COLOR;
	addItemText = "Add Item";

	if (useViewport)
	{
		container.addComponentListener(this);

		viewport.setViewedComponent(&container, false);
		viewport.setScrollBarsShown(true, false);
		viewport.setScrollOnDragEnabled(false);
		viewport.setScrollBarThickness(10);
		this->addAndMakeVisible(viewport);
	}

	BaseManager<T>* baseM = static_cast<BaseManager<T>*>(manager);
	baseM->addBaseManagerListener(this);
	baseM->addAsyncManagerListener(this);

	addItemBT = AssetManager::getInstance()->getAddBT();
	addAndMakeVisible(addItemBT);
	addItemBT->addListener(this);

	setShowAddButton(baseM->userCanAddItemsManually);

	if(Engine::mainEngine != nullptr) Engine::mainEngine->addEngineListener(this);

	//must call addExistingItems from child class to get overrides

	//setSize(100, 50); //default
}


template<class M, class T, class U>
BaseManagerUI<M, T, U>::~BaseManagerUI()
{
	if (!inspectable.wasObjectDeleted())
	{
		this->manager->removeBaseManagerListener(this);
		this->manager->removeAsyncManagerListener(this);
	}

	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::setDefaultLayout(Layout l)
{
	defaultLayout = l;
	if (useViewport)
	{
		if (defaultLayout == VERTICAL) viewport.setScrollBarsShown(true, false);
		else viewport.setScrollBarsShown(false, true);
	}

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::addExistingItems(bool resizeAfter)
{

	//add existing items
	for (auto &t : manager->items) addItemUI(t, false);
	if (resizeAfter) resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::setShowAddButton(bool value)
{
	addItemBT->setVisible(value);

	if (value)
	{
		addAndMakeVisible(addItemBT);
	} else
	{
		removeChildComponent(addItemBT);
	}

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::mouseDown(const MouseEvent & e)
{
	InspectableContentComponent::mouseDown(e);

	if (e.mods.isLeftButtonDown())
	{
	} else if (e.mods.isRightButtonDown())
	{
		if (manager->userCanAddItemsManually) showMenuAndAddItem(false, e.getEventRelativeTo(this).getMouseDownPosition());
	}
}

template<class M, class T, class U>
bool BaseManagerUI<M, T, U>::keyPressed(const KeyPress & e)
{
	if (InspectableContentComponent::keyPressed(e)) return true;

	if (e.getModifiers().isCommandDown())
	{
		/*
		if (e.getKeyCode() == KeyPress::createFromDescription("v").getKeyCode())
		{
			manager->addItemFromClipboard();
			return true;
		} else*/ 
		if (e.getKeyCode() == KeyPress::createFromDescription("a").getKeyCode())
		{
			int numItems = manager->items.size();
			if (numItems > 0) manager->items[0]->selectThis();
			if (numItems > 1) for (int i = 0; i < numItems; i++) manager->items[i]->selectThis(true);
		}
	} else
	{
		if ((e.getKeyCode() == e.deleteKey || e.getKeyCode() == e.backspaceKey) && manager->selectionManager == InspectableSelectionManager::activeSelectionManager)
		{
			Array<T *> itemsToRemove;
			for (auto & i : manager->selectionManager->currentInspectables)
			{
				T * it = dynamic_cast<T *>(i);
				if (it != nullptr) itemsToRemove.add(it);
			}

			manager->removeItems(itemsToRemove);
			return true;
		}
	}
	return false;
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::paint(Graphics & g)
{
	juce::Rectangle<int> r = getLocalBounds();

	if (!transparentBG)
	{
		g.setColour(bgColor);
		g.fillRoundedRectangle(r.toFloat(), 4);
	}

	if (drawContour)
	{

		Colour contourColor = bgColor.brighter(.2f);
		g.setColour(contourColor);
		g.drawRoundedRectangle(r.toFloat(), 4, 2);

		g.setFont(g.getCurrentFont().withHeight(labelHeight));
		float textWidth = g.getCurrentFont().getStringWidth(managerUIName);
		juce::Rectangle<int> tr = r.removeFromTop(labelHeight + 2).reduced((r.getWidth() - textWidth) / 2, 0).expanded(4, 0);
		g.fillRect(tr);
		Colour textColor = contourColor.withBrightness(contourColor.getBrightness() > .5f ? .1f : .9f).withAlpha(1.f);
		g.setColour(textColor);

		g.drawText(managerUIName, tr, Justification::centred, 1);
	} else
	{
		if (!transparentBG)	g.fillAll(bgColor);
	}

	if (grabbingItem != nullptr)
	{
		g.setColour(HIGHLIGHT_COLOR.withAlpha(.6f));
		g.fillRoundedRectangle(grabSpaceRect.translated(0, viewport.getY()).reduced(4).toFloat(), 2);
	}

	if (this->manager->items.size() == 0 && noItemText.isNotEmpty())
	{
		g.setColour(Colours::white.withAlpha(.4f));
		g.setFont(jmin(getHeight()-2,14));
		g.drawFittedText(noItemText, getLocalBounds().reduced(5), Justification::centred, 6);
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resized()
{
	if (getWidth() == 0 || getHeight() == 0) return;

	//bool resizeOnChange = resizeOnChildBoundsChanged;
	//resizeOnChildBoundsChanged = false; //avoir infinite loop if resize actually resizes inner components

	juce::Rectangle<int> r = getLocalBounds().reduced(2);
	resizedInternalHeader(r);
	resizedInternalFooter(r);
	resizedInternalContent(r);

	//resizeOnChildBoundsChanged = resizeOnChange;
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resizedInternalHeader(juce::Rectangle<int>& r)
{

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resizedInternalContent(juce::Rectangle<int>& r)
{

	if (addItemBT != nullptr && addItemBT->isVisible() && addItemBT->getParentComponent() == this)
	{
		addItemBT->setBounds(r.withSize(24, 24).withX(r.getWidth() - 24));
		r.removeFromTop(24);
	}


	if (useViewport)
	{
		viewport.setBounds(r);
		if (defaultLayout == VERTICAL) r.removeFromRight(drawContour ? 14 : 12);
		else r.removeFromBottom(drawContour ? 14 : 12);

		r.setY(0);
	}

	if (grabbingItem != nullptr)
	{
		int rel = defaultLayout == VERTICAL ? grabbingItem->getBottom() - viewport.getY() : grabbingItem->getRight() - viewport.getX();

		int ti = 0;
		for (auto &ui : itemsUI)
		{
			int tPos = defaultLayout == VERTICAL ? ui->getY() : ui->getX();
			if (tPos > rel)
			{
				grabbingItemDropIndex = ti;
				break;
			}
			ti++;
		}
		if (ti == itemsUI.size()) grabbingItemDropIndex = ti;
	}

	int i = 0;
	for (auto &ui : itemsUI)
	{
		BaseItemMinimalUI<T> * bui = static_cast<BaseItemMinimalUI<T>*>(ui);

		juce::Rectangle<int> tr;
		if (defaultLayout == VERTICAL)
		{
			if (grabbingItem != nullptr && i == grabbingItemDropIndex)
			{
				grabSpaceRect.setY(r.getY());
				r.translate(0, grabSpaceRect.getHeight() + gap);
			}
			tr = r.withHeight(bui->getHeight());
		} else
		{
			if (grabbingItem != nullptr && i == grabbingItemDropIndex)
			{
				grabSpaceRect.setX(r.getX());
				r.translate(grabSpaceRect.getWidth() + gap, 0);
			}
			tr = r.withWidth(bui->getWidth());
		}

		/*juce::Rectangle<int> vr = this->getLocalArea(&container, tr);
		if (defaultLayout == VERTICAL && (vr.getY() > viewport.getBounds().getBottom() - 20 || vr.getBottom() < viewport.getY() + 20))
		{
			bui->setVisible(false);
		} else
		{
			bui->setVisible(true);
		}*/

		if (tr != bui->getBounds()) bui->setBounds(tr);

		if (defaultLayout == VERTICAL) r.translate(0, tr.getHeight() + gap);
		else r.translate(tr.getWidth() + gap, 0);

		i++;
	}

	if (grabbingItemDropIndex >= itemsUI.size())
	{
		if (defaultLayout == VERTICAL)
		{
			grabSpaceRect.setY(r.getY());
			r.translate(0, grabSpaceRect.getHeight() + gap);
		} else
		{
			grabSpaceRect.setX(r.getX());
			r.translate(grabSpaceRect.getWidth() + gap, 0);
		}
	}

	if (grabbingItem != nullptr)
	{
		if (defaultLayout == VERTICAL) grabbingItem->setBounds(grabbingItem->getBounds().withY(grabbingItem->posAtDown.y + grabbingItem->dragOffset.y));
		else  grabbingItem->setBounds(grabbingItem->getBounds().withX(grabbingItem->posAtDown.x + grabbingItem->dragOffset.x));
	}


	if (useViewport || resizeOnChildBoundsChanged)
	{

		if (defaultLayout == VERTICAL)
		{
			float th = 0;
			if (itemsUI.size() > 0) th = static_cast<BaseItemMinimalUI<T>*>(itemsUI[itemsUI.size() - 1])->getBottom();
			if (grabbingItem != nullptr) th = jmax<int>(th + grabbingItem->getHeight(), viewport.getHeight());

			if (useViewport) container.setSize(getWidth(), th);
			else this->setSize(getWidth(), jmax<int>(th + 10, minHeight));
		} else if (defaultLayout == HORIZONTAL)
		{
			float tw = 0;
			if (itemsUI.size() > 0) tw = static_cast<BaseItemMinimalUI<T>*>(itemsUI[itemsUI.size() - 1])->getRight();
			if (grabbingItem != nullptr) tw = jmax<int>(tw, viewport.getWidth());
			if (useViewport) container.setSize(tw, getHeight());
			else this->setSize(tw, getHeight());
		}
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resizedInternalFooter(juce::Rectangle<int>& r)
{
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::updateItemsVisibility()
{ 
	for (auto &bui : itemsUI)
	{
		juce::Rectangle<int> vr = this->getLocalArea(bui, bui->getLocalBounds());
		if (defaultLayout == VERTICAL)
		{
			if (viewport.getHeight() > 0 && (vr.getY() > viewport.getBounds().getBottom() || vr.getBottom() < viewport.getY())) bui->setVisible(false);
			else bui->setVisible(true);
		}else
		{
			bui->setVisible(true);
		}
	}

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::childBoundsChanged(Component * c)
{
	if (resizeOnChildBoundsChanged && c != &viewport) resized();
}

template<class M, class T, class U>
bool BaseManagerUI<M, T, U>::hitTest(int x, int y)
{
	if (!autoFilterHitTestOnItems) return InspectableContentComponent::hitTest(x, y);
	if (itemsUI.size() == 0) return validateHitTestOnNoItem;

	for (auto &i : itemsUI)
	{
		Point<int> p(x, y);
		if (i->getBounds().contains(p))
		{
			Point<int> localP = i->getLocalPoint(this, p);
			if (i->hitTest(localP.x, localP.y)) return true;
		}
	}
			
	return false;
}

template<class M, class T, class U>
inline void BaseManagerUI<M, T, U>::componentMovedOrResized(Component & c, bool wasMoved, bool wasResized)
{
	if (&c == &container && useViewport && !itemAnimator.isAnimating())
	{
		resized();
		updateItemsVisibility();
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::showMenuAndAddItem(bool isFromAddButton, Point<int> mouseDownPos)
{
	if (manager->managerFactory != nullptr)
	{
		T * m = manager->managerFactory->showCreateMenu();
		if (m != nullptr) ((BaseManager<T> *)manager)->addItem(m);
	} else
	{
		if (isFromAddButton)
		{
			manager->BaseManager<T>::addItem();
			return;
		}

		PopupMenu p;
		p.addItem(1, addItemText);

		int result = p.show();
		switch (result)
		{
		case 1:
			addItemFromMenu(isFromAddButton, mouseDownPos);
			break;
		}
	}

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::addItemFromMenu(bool, Point<int>)
{
	manager->BaseManager<T>::addItem();
}

template<class M, class T, class U>
U * BaseManagerUI<M, T, U>::addItemUI(T * item, bool animate, bool resizeAndRepaint)
{
	U * tui = createUIForItem(item);

	BaseItemMinimalUI<T> * bui = static_cast<BaseItemMinimalUI<T>*>(tui);

	if (useViewport) container.addAndMakeVisible(bui);
	else addAndMakeVisible(bui);

	itemsUI.insert(manager->items.indexOf(item), tui);

	BaseItemUI<T> * biui = dynamic_cast<BaseItemUI<T> *>(tui);
	if (biui != nullptr) biui->addItemUIListener(this);


	addItemUIInternal(tui);

	if (animate)
	{
		juce::Rectangle<int> tb = bui->getBounds();
		bui->setSize(10, 10);
		itemAnimator.animateComponent(bui, tb, 1, 200, false, 1, 0);
	} else
	{
		//DBG("resized");  
		//resized();
	}

	managerUIListeners.call(&ManagerUIListener::itemUIAdded, tui);

	repaint();

	return tui;
}

template<class M, class T, class U>
inline U * BaseManagerUI<M, T, U>::createUIForItem(T * item)
{
	return new U(item);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::removeItemUI(T * item, bool resizeAndRepaint)
{
	MessageManagerLock mmLock; //Ensure this method can be called from another thread than the UI one

	U * tui = getUIForItem(item, false);
	if (tui == nullptr) return;

	if (useViewport) container.removeChildComponent(static_cast<BaseItemMinimalUI<T>*>(tui));
	else removeChildComponent(static_cast<BaseItemMinimalUI<T>*>(tui));


	BaseItemUI<T> * biui = dynamic_cast<BaseItemUI<T> *>(tui);
	if (biui != nullptr) biui->removeItemUIListener(this);

	itemsUI.removeObject(tui, false);
	removeItemUIInternal(tui);

	managerUIListeners.call(&ManagerUIListener::itemUIRemoved, tui);

	delete tui;

	if (resizeAndRepaint)
	{
		resized();
		updateItemsVisibility();
		repaint();
	}
}

template<class M, class T, class U>
U * BaseManagerUI<M, T, U>::getUIForItem(T * item, bool directIndexAccess)
{
	if (directIndexAccess) return itemsUI[static_cast<BaseManager<T>*>(manager)->items.indexOf(item)];

	for (auto &ui : itemsUI) if (static_cast<BaseItemMinimalUI<T>*>(ui)->item == item) return ui; //brute search, not needed if ui/items are synchronized
	return nullptr;
}

template<class M, class T, class U>
int BaseManagerUI<M, T, U>::getContentHeight()
{
	return container.getHeight() + 20;
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemAddedAsync(T * item)
{
	addItemUI(item, animateItemOnAdd);
	if (!animateItemOnAdd) resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemsAddedAsync(Array<T*> items)
{
	for (auto &i : items) addItemUI(i, false, false);
	
	resized();
	repaint();
	
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemRemoved(T * item)
{
	removeItemUI(item);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemsRemoved(Array<T*> items)
{
	if (items.size() == 0) return;

	for (auto &i : items) removeItemUI(i,false);

	MessageManagerLock mmLock;
	if (mmLock.lockWasGained())
	{
		resized();
		repaint();
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemsReorderedAsync()
{
	itemsUI.sort(managerComparator);
	resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::newMessage(const typename BaseManager<T>::ManagerEvent & e)
{
	switch (e.type)
	{

	case BaseManager<T>::ManagerEvent::ITEM_ADDED:
		itemAddedAsync(e.getItem());
		break;

	case BaseManager<T>::ManagerEvent::ITEMS_REORDERED:
		itemsReorderedAsync();
		break;

	case BaseManager<T>::ManagerEvent::ITEMS_ADDED:
		itemsAddedAsync(e.getItems());
		break;

	default:
		break;
	}
}


template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemUIGrabStart(BaseItemUI<T>* se)
{
	grabbingItem = se;

	grabbingItemDropIndex = itemsUI.indexOf((U*)grabbingItem);
	grabSpaceRect = ((U *)grabbingItem)->getBounds();
	itemsUI.removeObject((U *)grabbingItem, false);
	grabbingItem->toFront(false);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemUIGrabbed(BaseItemUI<T>* se)
{
	if (grabbingItem != nullptr) resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemUIGrabEnd(BaseItemUI<T>* se)
{
	if (grabbingItem != nullptr)
	{

		int targetIndex = grabbingItemDropIndex;
		itemsUI.insert(targetIndex,(U *)grabbingItem);

		grabbingItem = nullptr;
		grabbingItemDropIndex = -1;

		manager->setItemIndex(se->item, targetIndex);
		resized();
	}
}


template<class M, class T, class U>
void BaseManagerUI<M, T, U>::buttonClicked(Button  * b)
{
	if (b == addItemBT)
	{
		showMenuAndAddItem(true, Point<int>());
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::inspectableDestroyed(Inspectable *)
{
	if (manager != nullptr) static_cast<BaseManager<T>*>(manager)->removeBaseManagerListener(this);
}

template<class M, class T, class U>
inline void BaseManagerUI<M, T, U>::startLoadFile()
{
	tmpAnimate = animateItemOnAdd;
	animateItemOnAdd = false;
}

template<class M, class T, class U>
inline void BaseManagerUI<M, T, U>::endLoadFile()
{
	animateItemOnAdd = tmpAnimate;
	resized();
}


#endif  // BASEMANAGERUI_H_INCLUDED
