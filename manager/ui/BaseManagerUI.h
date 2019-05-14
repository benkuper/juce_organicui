/*
  ==============================================================================

	BaseManagerUI.h
	Created: 28 Oct 2016 8:03:45pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once
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
	public BaseItemMinimalUI<T>::ItemMinimalUIListener,
	public ComponentListener,
	public DragAndDropTarget
{
public:
	BaseManagerUI<M, T, U>(const String &contentName, M * _manager, bool _useViewport = true);
	virtual ~BaseManagerUI();

	enum Layout { HORIZONTAL, VERTICAL, FREE };

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
	/*
	BaseItemMinimalUI<T> * grabbingItem;
	int grabbingItemDropIndex;
	juce::Rectangle<int> grabSpaceRect;
	*/

	//drag and drop
	StringArray acceptedDropTypes;
	bool isDraggingOver;
	bool highlightOnDragOver;
	int currentDropIndex;

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
	virtual void addItemFromMenu(T * item, bool isFromAddButton, Point<int> mouseDownPos);

	virtual U * addItemUI(T * item, bool animate = false, bool resizeAndRepaint = true);
	virtual U * createUIForItem(T * item);
	virtual void addItemUIInternal(U *) {}

	virtual void mouseDown(const MouseEvent &e) override;

	virtual void removeItemUI(T * item, bool resizeAndRepaint = true);
	virtual void removeItemUIInternal(U *) {}

	 
	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails & dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragMove(const SourceDetails& dragSourceDetails) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails & dragSourceDetails) override;

	virtual int getDropIndexForPosition(Point<int> localPosition);
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
	isDraggingOver(false),
	highlightOnDragOver(true),
	fixedItemHeight(true),
	gap(2)
{
	
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
	
	acceptedDropTypes.add(baseM->itemDataType);

	//must call addExistingItems from child class to get overrides
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

	if (isDraggingOver && highlightOnDragOver)
	{
		g.setColour(BLUE_COLOR);
		
		switch (defaultLayout)
		{
		case HORIZONTAL:
		case VERTICAL:
		{
			if (itemsUI.size() > 0)
			{
				BaseItemMinimalUI<T> * bui = dynamic_cast<BaseItemMinimalUI<T> *>(itemsUI[currentDropIndex >= 0 ? currentDropIndex : itemsUI.size() - 1]);
				if (bui != nullptr)
				{
					juce::Rectangle<int> buiBounds = getLocalArea(bui, bui->getLocalBounds());
					if (defaultLayout == HORIZONTAL)
					{
						int tx = currentDropIndex >= 0 ? buiBounds.getX() - 1 : buiBounds.getRight() + 1;
						g.drawLine(tx, 0, tx, getHeight(), 2);
					}
					else if (defaultLayout == VERTICAL)
					{
						int ty = currentDropIndex >= 0 ? buiBounds.getY() - 1 : buiBounds.getBottom() + 1;
						g.drawLine(0,ty , getWidth(), ty, 2);
					}
				}
			}
		}
		break;

		case FREE:
			g.drawRoundedRectangle(r.toFloat(), 4, 2);
			break;
		}
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

	int i = 0;
	for (auto &ui : itemsUI)
	{
		BaseItemMinimalUI<T> * bui = static_cast<BaseItemMinimalUI<T>*>(ui);

		juce::Rectangle<int> tr;
		if (defaultLayout == VERTICAL)
		{
			tr = r.withHeight(bui->getHeight());
		} else
		{


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

	if (useViewport || resizeOnChildBoundsChanged)
	{

		if (defaultLayout == VERTICAL)
		{
			float th = 0;
			if (itemsUI.size() > 0) th = static_cast<BaseItemMinimalUI<T>*>(itemsUI[itemsUI.size() - 1])->getBottom();
			//if (grabbingItem != nullptr) th = jmax<int>(th + grabbingItem->getHeight(), viewport.getHeight());

			if (useViewport) container.setSize(getWidth(), th);
			else this->setSize(getWidth(), jmax<int>(th + 10, minHeight));
		} else if (defaultLayout == HORIZONTAL)
		{
			float tw = 0;
			if (itemsUI.size() > 0) tw = static_cast<BaseItemMinimalUI<T>*>(itemsUI[itemsUI.size() - 1])->getRight();
			//if (grabbingItem != nullptr) tw = jmax<int>(tw, viewport.getWidth());
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
		T * item = manager->managerFactory->showCreateMenu();
		if (item != nullptr) addItemFromMenu(item, isFromAddButton, mouseDownPos);
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
void BaseManagerUI<M, T, U>::addItemFromMenu(T * item, bool, Point<int>)
{
	manager->BaseManager<T>::addItem(item);
}

template<class M, class T, class U>
U * BaseManagerUI<M, T, U>::addItemUI(T * item, bool animate, bool resizeAndRepaint)
{
	U * tui = createUIForItem(item);

	BaseItemMinimalUI<T> * bui = static_cast<BaseItemMinimalUI<T>*>(tui);

	if (useViewport) container.addAndMakeVisible(bui);
	else addAndMakeVisible(bui);

	itemsUI.insert(manager->items.indexOf(item), tui);
	
	bui->addItemMinimalUIListener(this);

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

	BaseItemMinimalUI<T> * bui = static_cast<BaseItemMinimalUI<T>*>(tui);

	if (useViewport) container.removeChildComponent(bui);
	else removeChildComponent(bui);

	bui->removeItemMinimalUIListener(this);

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
bool BaseManagerUI<M, T, U>::isInterestedInDragSource(const SourceDetails & dragSourceDetails)
{
	if (acceptedDropTypes.contains(dragSourceDetails.description.getProperty("dataType", "").toString())) return true;

	U * itemUI = dynamic_cast<U *>(dragSourceDetails.sourceComponent.get());
	if (itemsUI.contains(itemUI)) return true;

	return false;
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	if (highlightOnDragOver) repaint();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemDragMove(const SourceDetails &dragSourceDetails)
{
	if (defaultLayout == HORIZONTAL || defaultLayout == VERTICAL)
	{
		currentDropIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
		repaint();
	}
}


template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemDragExit(const SourceDetails&)
{
	isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemDropped(const SourceDetails & dragSourceDetails)
{
	
	if (defaultLayout == HORIZONTAL || defaultLayout == VERTICAL)
	{
		BaseItemMinimalUI<T> * bui = dynamic_cast<BaseItemMinimalUI<T> *>(dragSourceDetails.sourceComponent.get());
		if (bui != nullptr)
		{
			int droppingIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
			if (itemsUI.contains((U *)bui))
			{
				if (itemsUI.indexOf((U*)bui) < droppingIndex) droppingIndex--;
				if (droppingIndex == -1) droppingIndex = itemsUI.size() - 1;
				this->manager->setItemIndex(bui->item, droppingIndex);
			}
			else
			{
				var data = bui->item->getJSONData();
				T * newItem = manager->addItemFromData(data);
				if (newItem != nullptr)
				{
					if (droppingIndex == -1) droppingIndex = itemsUI.size() - 1;
					this->manager->setItemIndex(newItem, droppingIndex);
					bui->item->remove();
				}
			}
		}
	}

	this->isDraggingOver = false;
	if (highlightOnDragOver) repaint();
}

template<class M, class T, class U>
int BaseManagerUI<M, T, U>::getDropIndexForPosition(Point<int> localPosition)
{
	for (int i = 0; i < itemsUI.size(); i++)
	{
		BaseItemMinimalUI<T> * iui = dynamic_cast<BaseItemMinimalUI<T> *>(itemsUI[i]);
		Point<int> p = getLocalArea(iui, iui->getLocalBounds()).getCentre();

		if (defaultLayout == HORIZONTAL && localPosition.x < p.x) return i;
		else if (defaultLayout == VERTICAL && localPosition.y < p.y) return i;
	}

	return -1;
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