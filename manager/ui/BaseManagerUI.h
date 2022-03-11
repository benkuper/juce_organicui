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
	BaseManagerItemComparator(BaseManager<IT>* _manager) :manager(_manager) {}

	BaseManager<IT>* manager;

	int compareElements(BaseItemMinimalUI<IT>* u1, BaseItemMinimalUI<IT>* u2)
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
	ManagerUIItemContainer<M, T, U>(BaseManagerUI<M, T, U>* _mui) : mui(_mui) {};
	~ManagerUIItemContainer() {}

	BaseManagerUI<M, T, U>* mui;

	void childBoundsChanged(Component* c) { mui->childBoundsChanged(c); }
};



template<class M, class T, class U>
class BaseManagerUI :
	public InspectableContentComponent,
	public BaseManager<T>::ManagerListener,
	public BaseManager<T>::AsyncListener,
	public Button::Listener,
	public BaseItemUI<T>::ItemUIListener,
	public BaseItemMinimalUI<T>::ItemMinimalUIListener,
	public ComponentListener,
	public Engine::AsyncListener,
	public DragAndDropTarget,
	public Label::Listener
{
public:
	BaseManagerUI<M, T, U>(const String& contentName, M* _manager, bool _useViewport = true);
	virtual ~BaseManagerUI();

	enum Layout { HORIZONTAL, VERTICAL, FREE };

	M* manager;
	OwnedArray<U> itemsUI;
	BaseManagerItemComparator<T> managerComparator;

	//ui
	bool useViewport; //TODO, create a BaseManagerViewportUI

	Layout defaultLayout;

	ManagerUIItemContainer<M, T, U> container;
	Viewport viewport;

	int headerSize;
	Colour bgColor;
	int labelHeight;
	int minHeight;
	String managerUIName;
	bool drawContour;
	bool transparentBG;
	bool resizeOnChildBoundsChanged;
	bool autoFilterHitTestOnItems;
	bool validateHitTestOnNoItem;

	std::unique_ptr<ImageButton> addItemBT;
	std::unique_ptr<Label> searchBar;

	OwnedArray<Component> tools;
	HashMap<Button*, std::function<void()>> toolFuncMap;
	bool showTools;

	//ui
	String noItemText;

	//menu
	String addItemText;

	//Animation
	bool animateItemOnAdd;
	ComponentAnimator itemAnimator;

	//selection
	bool useDedicatedSelector;
	bool selectingItems;

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
	void setShowSearchBar(bool value);

	//tools
	void setShowTools(bool value);
	void addButtonTool(Button* c, std::function<void()> clickFunc = nullptr);
	void addControllableTool(ControllableUI* c);

	virtual void paint(Graphics& g) override;

	virtual void resized() override;
	virtual juce::Rectangle<int> setHeaderBounds(juce::Rectangle<int>& r);
	virtual void resizedInternalHeader(juce::Rectangle<int>& r);
	virtual void resizedInternalHeaderTools(juce::Rectangle<int>& r);
	virtual void resizedInternalContent(juce::Rectangle<int>& r);
	virtual void placeItems(juce::Rectangle<int>& r);
	virtual void resizedInternalFooter(juce::Rectangle<int>& r);

	enum AlignMode { LEFT, RIGHT, TOP, BOTTOM, CENTER_H, CENTER_V };
	virtual void alignItems(AlignMode alignMode);

	class PositionComparator
	{
	public:
		PositionComparator(bool isVertical) : isVertical(isVertical) {}
		bool isVertical;
		int compareElements(T* i1, T* i2) {
			return isVertical ? i1->viewUIPosition->y > i2->viewUIPosition->y : i1->viewUIPosition->x > i2->viewUIPosition->x;
		}
	};
	virtual void distributeItems(bool isVertical);

	virtual void updateItemsVisibility();
	virtual void updateItemVisibility(U* bui);

	virtual bool hasFiltering();
	virtual Array<U*> getFilteredItems();
	virtual bool checkFilterForItem(U* item);

	virtual void childBoundsChanged(Component*) override;

	virtual bool hitTest(int x, int y) override;

	//For container check
	virtual void componentMovedOrResized(Component& c, bool wasMoved, bool wasResized) override;


	virtual void showMenuAndAddItem(bool isFromAddButton, Point<int> mouseDownPos);
	virtual void addMenuExtraItems(PopupMenu& p, int startIndex) {}
	virtual void handleMenuExtraItemsResult(int result, int startIndex) {}
	virtual void addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos);
	virtual void addItemFromMenu(T* item, bool isFromAddButton, Point<int> mouseDownPos);

	virtual U* addItemUI(T* item, bool animate = false, bool resizeAndRepaint = true);
	virtual U* createUIForItem(T* item);
	virtual void addItemUIInternal(U*) {}

	virtual void mouseDown(const MouseEvent& e) override;
	virtual void mouseUp(const MouseEvent& e) override;

	virtual void askSelectToThis(BaseItemMinimalUI<T>* item) override;

	virtual void removeItemUI(T* item, bool resizeAndRepaint = true);
	virtual void removeItemUIInternal(U*) {}

	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragMove(const SourceDetails& dragSourceDetails) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails& dragSourceDetails) override;

	//Selection
	virtual void addSelectableComponentsAndInspectables(Array<Component*>& selectables, Array<Inspectable*>& inspectables);
	virtual Component* getSelectableComponentForItemUI(U* itemUI);

	virtual int getDropIndexForPosition(Point<int> localPosition);
	virtual void itemUIMiniModeChanged(BaseItemUI<T>* se) override {}

	//menu
	U* getUIForItem(T* item, bool directIndexAccess = true);

	int getContentHeight();

	virtual void itemAddedAsync(T* item);
	virtual void itemsAddedAsync(Array<T*> items);

	virtual void itemRemoved(T* item) override; //must keep this one realtime because the async may cause the target item to already be deleted by the time this function is called
	virtual void itemsRemoved(Array<T*> items) override;
	virtual void itemsReorderedAsync();

	void newMessage(const typename BaseManager<T>::ManagerEvent& e) override;

	void buttonClicked(Button* b) override;
	void labelTextChanged(Label* l) override;

	virtual void inspectableDestroyed(Inspectable*) override;

	virtual void newMessage(const Engine::EngineEvent& e) override;

	class  ManagerUIListener
	{
	public:
		/** Destructor. */
		virtual ~ManagerUIListener() {}
		virtual void itemUIAdded(U*) {}
		virtual void itemUIRemoved(U*) {}
	};

	ListenerList<ManagerUIListener> managerUIListeners;
	void addManagerUIListener(ManagerUIListener* newListener) { managerUIListeners.add(newListener); }
	void removeManagerUIListener(ManagerUIListener* listener) { managerUIListeners.remove(listener); }
};


template<class M, class T, class U>
BaseManagerUI<M, T, U>::BaseManagerUI(const String& contentName, M* _manager, bool _useViewport) :
	InspectableContentComponent(_manager),
	manager(_manager),
	managerComparator(_manager),
	useViewport(_useViewport),
	defaultLayout(VERTICAL),
	container(this),
	headerSize(24),
	bgColor(BG_COLOR),
	labelHeight(10),
	minHeight(50),
	managerUIName(contentName),
	drawContour(false),
	transparentBG(false),
	resizeOnChildBoundsChanged(true),
	autoFilterHitTestOnItems(false),
	validateHitTestOnNoItem(true),
	showTools(false),
	animateItemOnAdd(true),
	useDedicatedSelector(true),
	selectingItems(false),
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
		viewport.setScrollOnDragMode(Viewport::ScrollOnDragMode::never);
		viewport.setScrollBarThickness(10);
		viewport.setEnableKeyPressEvents(false);
		this->addAndMakeVisible(viewport);
	}

	BaseManager<T>* baseM = static_cast<BaseManager<T>*>(manager);
	baseM->addBaseManagerListener(this);
	baseM->addAsyncManagerListener(this);

	addItemBT.reset(AssetManager::getInstance()->getAddBT());
	addItemBT->setWantsKeyboardFocus(false);

	addAndMakeVisible(addItemBT.get());
	addItemBT->addListener(this);

	setShowAddButton(baseM->userCanAddItemsManually);

	acceptedDropTypes.add(baseM->itemDataType);

	Engine::mainEngine->addAsyncEngineListener(this);

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

	Engine::mainEngine->removeAsyncEngineListener(this);
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
	for (auto& t : manager->items) addItemUI(t, false);
	if (resizeAfter) resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::setShowAddButton(bool value)
{
	addItemBT->setVisible(value);

	if (value) addAndMakeVisible(addItemBT.get());
	else removeChildComponent(addItemBT.get());
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::setShowSearchBar(bool value)
{
	if (value)
	{
		searchBar.reset(new Label("SearchBar"));
		searchBar->setJustificationType(Justification::topLeft);
		searchBar->setColour(searchBar->backgroundColourId, BG_COLOR.darker(.1f).withAlpha(.7f));
		searchBar->setColour(searchBar->outlineColourId, BG_COLOR.brighter(.1f));
		searchBar->setColour(searchBar->textColourId, TEXT_COLOR.darker(.3f));
		searchBar->setFont(10);
		searchBar->setColour(CaretComponent::caretColourId, Colours::orange);
		searchBar->setEditable(true);
		searchBar->addListener(this);
		addAndMakeVisible(searchBar.get());
	}
	else if (searchBar != nullptr)
	{
		removeChildComponent(searchBar.get());
		searchBar.reset();
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::setShowTools(bool value)
{
	if (value == showTools) return;

	showTools = value;
	if (value)
	{
		for (auto& c : tools) addAndMakeVisible(c);
	}
	else
	{
		for (auto& c : tools) removeChildComponent(c);
	}

	resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::addButtonTool(Button* c, std::function<void()> clickFunc)
{
	if (tools.contains(c)) return;
	tools.add(c);
	if (clickFunc != nullptr) toolFuncMap.set(c, clickFunc);
	c->addListener(this);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::addControllableTool(ControllableUI* c)
{
	tools.add(c);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::mouseDown(const MouseEvent& e)
{
	InspectableContentComponent::mouseDown(e);

	if (e.eventComponent == this)
	{
		if (e.mods.isLeftButtonDown())
		{
			if (!e.mods.isCommandDown() && !e.mods.isAltDown())
			{
				if (useDedicatedSelector)
				{
					Array<Component*> selectables;
					Array<Inspectable*> inspectables;
					addSelectableComponentsAndInspectables(selectables, inspectables);

					InspectableSelector::getInstance()->startSelection(this, selectables, inspectables, nullptr, !e.mods.isShiftDown());
					selectingItems = true;
				}
			}
		}
		else if (e.mods.isRightButtonDown() && e.eventComponent == this)
		{
			if (manager->userCanAddItemsManually) showMenuAndAddItem(false, e.getEventRelativeTo(this).getMouseDownPosition());
		}
	}

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::mouseUp(const MouseEvent& e)
{
	if (e.eventComponent == this)
	{
		if (selectingItems)
		{
			//InspectableSelector::getInstance()->endSelection();
			selectingItems = false;
		}
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::askSelectToThis(BaseItemMinimalUI<T>* itemUI)
{
	T* firstItem = InspectableSelectionManager::activeSelectionManager->getInspectableAs<T>();
	int firstIndex = manager->items.indexOf(firstItem);
	int itemIndex = manager->items.indexOf(itemUI->item);

	if (firstIndex == itemIndex) return;

	if (firstIndex >= 0 && itemIndex >= 0)
	{
		int step = firstIndex < itemIndex ? 1 : -1;

		for (int index = firstIndex; index != itemIndex; index += step)
		{
			manager->items[index]->selectThis(true);
		}

	}

	if (itemIndex >= 0) manager->items[itemIndex]->selectThis(true);
}


template<class M, class T, class U>
void BaseManagerUI<M, T, U>::paint(Graphics& g)
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
	}
	else
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
				BaseItemMinimalUI<T>* bui = dynamic_cast<BaseItemMinimalUI<T>*>(itemsUI[currentDropIndex >= 0 ? currentDropIndex : itemsUI.size() - 1]);
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
						g.drawLine(0, ty, getWidth(), ty, 2);
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
		g.setFont(jmin(getHeight() - 2, 14));
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
	juce::Rectangle<int> hr = setHeaderBounds(r);
	resizedInternalHeader(hr);
	resizedInternalFooter(r);
	resizedInternalContent(r);

	updateItemsVisibility();

	//resizeOnChildBoundsChanged = resizeOnChange;
}

template<class M, class T, class U>
juce::Rectangle<int> BaseManagerUI<M, T, U>::setHeaderBounds(juce::Rectangle<int>& r)
{
	return defaultLayout == VERTICAL ? r.removeFromTop(headerSize) : r.removeFromRight(headerSize);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resizedInternalHeader(juce::Rectangle<int>& hr)
{
	if (addItemBT != nullptr && addItemBT->isVisible() && addItemBT->getParentComponent() == this)
	{
		if (defaultLayout == VERTICAL || defaultLayout == FREE) addItemBT->setBounds(hr.removeFromRight(hr.getHeight()).reduced(2));
		else addItemBT->setBounds(hr.removeFromTop(24).removeFromRight(24).reduced(2));
	}

	if (searchBar != nullptr && searchBar->isVisible() && searchBar->getParentComponent() == this)
	{
		if (defaultLayout == VERTICAL || defaultLayout == FREE) searchBar->setBounds(hr.removeFromLeft(150).reduced(2));
		else searchBar->setBounds(hr.removeFromTop(20).reduced(2));
	}

	if (showTools) resizedInternalHeaderTools(hr);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resizedInternalHeaderTools(juce::Rectangle<int>& r)
{
	r.removeFromLeft(4);
	
	float rSize = 0;
	for (auto& t : this->tools) rSize += jmax(t->getWidth(), r.getHeight());

	r.removeFromLeft((r.getWidth() - rSize) / 2);

	if (r.getWidth() == 0 || r.getHeight() == 0) return;

	for (auto& t : this->tools)
	{
		juce::Rectangle<int> tr = r.removeFromLeft(jmax(t->getWidth(), r.getHeight()));
		tr.reduce(tr.getWidth() == r.getHeight() ? 6 : 0, 6);
		t->setBounds(tr);
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resizedInternalContent(juce::Rectangle<int>& r)
{
	if (useViewport)
	{
		viewport.setBounds(r);
		if (defaultLayout == VERTICAL) r.removeFromRight(drawContour ? 14 : 12);
		else r.removeFromBottom(drawContour ? 14 : 12);

		r.setY(0);
	}

	placeItems(r);

	if (useViewport || resizeOnChildBoundsChanged)
	{
		if (defaultLayout == VERTICAL)
		{
			float th = 0;
			if (itemsUI.size() > 0) th = static_cast<BaseItemMinimalUI<T>*>(itemsUI[itemsUI.size() - 1])->getBottom();
			//if (grabbingItem != nullptr) th = jmax<int>(th + grabbingItem->getHeight(), viewport.getHeight());

			if (useViewport) container.setSize(getWidth(), th);
			else this->setSize(getWidth(), jmax<int>(th + 10, minHeight));
		}
		else if (defaultLayout == HORIZONTAL)
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
void BaseManagerUI<M, T, U>::placeItems(juce::Rectangle<int>& r)
{
	int i = 0;
	for (auto& ui : itemsUI)
	{
		BaseItemMinimalUI<T>* bui = static_cast<BaseItemMinimalUI<T>*>(ui);
		if (!checkFilterForItem(ui))
		{
			bui->setVisible(false);
			continue;
		}
		bui->setVisible(true);

		juce::Rectangle<int> tr;
		if (defaultLayout == VERTICAL) tr = r.withHeight(bui->getHeight());
		else tr = r.withWidth(bui->getWidth());

		if (tr != bui->getBounds()) bui->setBounds(tr);

		if (defaultLayout == VERTICAL) r.translate(0, tr.getHeight() + gap);
		else r.translate(tr.getWidth() + gap, 0);

		++i;
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::resizedInternalFooter(juce::Rectangle<int>& r)
{
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::alignItems(AlignMode alignMode)
{
	Array<T*> inspectables = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<T>();
	if (inspectables.size() == 0) return;

	float target = (alignMode == BaseManagerUI<M, T, U>::AlignMode::CENTER_V || alignMode == BaseManagerUI<M, T, U>::AlignMode::CENTER_H) ? 0 : ((alignMode == BaseManagerUI<M, T, U>::AlignMode::LEFT || alignMode == BaseManagerUI<M, T, U>::AlignMode::TOP) ? INT_MAX : INT_MIN);

	Array<T*> goodInspectables;
	for (auto& i : inspectables)
	{
		if (i == nullptr || !manager->items.contains(i)) continue;
		switch (alignMode)
		{
		case BaseManagerUI<M, T, U>::AlignMode::LEFT: target = jmin(i->viewUIPosition->x, target); break;
		case BaseManagerUI<M, T, U>::AlignMode::RIGHT: target = jmax(i->viewUIPosition->x + i->viewUISize->x, target); break;
		case BaseManagerUI<M, T, U>::AlignMode::CENTER_H: target += i->viewUIPosition->x + i->viewUISize->x / 2; break;
		case BaseManagerUI<M, T, U>::AlignMode::TOP: target = jmin(target, i->viewUIPosition->y); break;
		case BaseManagerUI<M, T, U>::AlignMode::BOTTOM: target = jmax(target, i->viewUIPosition->y + i->viewUISize->y); break;
		case BaseManagerUI<M, T, U>::AlignMode::CENTER_V: target += i->viewUIPosition->y + i->viewUISize->y / 2; break;
		}
		goodInspectables.add(i);
	}

	if (goodInspectables.size() == 0) return;

	if (alignMode == CENTER_H || alignMode == CENTER_V) target /= goodInspectables.size();

	Array<UndoableAction*> actions;
	for (auto& i : goodInspectables)
	{
		Point<float> targetPoint;
		switch (alignMode)
		{
		case BaseManagerUI<M, T, U>::AlignMode::LEFT: targetPoint = Point<float>(target, i->viewUIPosition->y); break;
		case BaseManagerUI<M, T, U>::AlignMode::RIGHT: targetPoint = Point<float>(target - i->viewUISize->x, i->viewUIPosition->y); break;
		case BaseManagerUI<M, T, U>::AlignMode::CENTER_H: targetPoint = Point<float>(target - i->viewUISize->x / 2, i->viewUIPosition->y);  break;
		case BaseManagerUI<M, T, U>::AlignMode::TOP: targetPoint = Point<float>(i->viewUIPosition->x, target); break;
		case BaseManagerUI<M, T, U>::AlignMode::BOTTOM:targetPoint = Point<float>(i->viewUIPosition->x, target - i->viewUISize->y); break;
		case BaseManagerUI<M, T, U>::AlignMode::CENTER_V:targetPoint = Point<float>(i->viewUIPosition->x, target - i->viewUISize->y / 2); break;
		}
		actions.add(i->viewUIPosition->setUndoablePoint(i->viewUIPosition->getPoint(), targetPoint, true));
	}

	UndoMaster::getInstance()->performActions("Align " + String(goodInspectables.size() + " items"), actions);

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::distributeItems(bool isVertical)
{
	Array<T*> inspectables = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<T>();
	if (inspectables.size() < 3) return;


	Array<T*> goodInspectables;
	for (auto& i : inspectables)
	{
		if (i == nullptr || !manager->items.contains(i)) continue;

		goodInspectables.add(i);
	}

	if (goodInspectables.size() < 3) return;


	PositionComparator comp(isVertical);
	goodInspectables.sort(comp);

	Point<float> center0 = goodInspectables[0]->viewUIPosition->getPoint() + goodInspectables[0]->viewUISize->getPoint() / 2;
	Point<float> center1 = goodInspectables[goodInspectables.size() - 1]->viewUIPosition->getPoint() + goodInspectables[goodInspectables.size() - 1]->viewUISize->getPoint() / 2;
	float tMin = isVertical ? center0.y : center0.x;
	float tMax = isVertical ? center1.y : center1.x;

	Array<UndoableAction*> actions;
	for (int i = 0; i < goodInspectables.size(); i++)
	{
		float rel = i * 1.0f / (goodInspectables.size() - 1);
		float target = jmap(rel, tMin, tMax);
		T* ti = goodInspectables[i];
		Point<float> targetPoint(isVertical ? ti->viewUIPosition->x : target - ti->viewUISize->x / 2, isVertical ? target - ti->viewUISize->y / 2 : ti->viewUIPosition->y);
		actions.add(ti->viewUIPosition->setUndoablePoint(ti->viewUIPosition->getPoint(), targetPoint, true));
	}

	UndoMaster::getInstance()->performActions("Distribute " + String(goodInspectables.size() + " items"), actions);
}


template<class M, class T, class U>
void BaseManagerUI<M, T, U>::updateItemsVisibility()
{
	for (auto& bui : itemsUI) updateItemVisibility(bui);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::updateItemVisibility(U* bui)
{
	if (!checkFilterForItem(bui)) return;

	juce::Rectangle<int> vr = this->getLocalArea(bui, bui->getLocalBounds());
	if (defaultLayout == VERTICAL)
	{
		if (viewport.getHeight() > 0 && (vr.getY() > viewport.getBounds().getBottom() || vr.getBottom() < viewport.getY())) bui->setVisible(false);
		else bui->setVisible(true);
	}
	else
	{
		bui->setVisible(true);
	}
}

template<class M, class T, class U>
bool BaseManagerUI<M, T, U>::hasFiltering()
{
	return searchBar != nullptr && searchBar->getText().isNotEmpty();
}

template<class M, class T, class U>
Array<U*> BaseManagerUI<M, T, U>::getFilteredItems()
{
	if (!this->hasFiltering()) return Array<U*>(this->itemsUI.getRawDataPointer(), this->itemsUI.size());

	Array<U*> result;
	for (auto& ui : this->itemsUI) if (checkFilterForItem(ui)) result.add(ui);
	return result;
}

template<class M, class T, class U>
bool BaseManagerUI<M, T, U>::checkFilterForItem(U* ui)
{
	if (!this->hasFiltering() || searchBar == nullptr) return true;
	return ui->item->niceName.toLowerCase().contains(searchBar->getText().toLowerCase());
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::childBoundsChanged(Component* c)
{
	if (resizeOnChildBoundsChanged && c != &viewport) resized();
}

template<class M, class T, class U>
bool BaseManagerUI<M, T, U>::hitTest(int x, int y)
{
	if (!autoFilterHitTestOnItems) return InspectableContentComponent::hitTest(x, y);
	if (itemsUI.size() == 0) return validateHitTestOnNoItem;

	Point<int> p(x, y);
	for (auto& i : itemsUI)
	{
		if (i->getBounds().contains(p))
		{
			Point<int> localP = i->getLocalPoint(this, p);
			if (i->hitTest(localP.x, localP.y)) return true;
		}
	}

	return false;
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::componentMovedOrResized(Component& c, bool wasMoved, bool wasResized)
{
	if (&c == &container && useViewport && !itemAnimator.isAnimating())
	{
		resized();
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::showMenuAndAddItem(bool isFromAddButton, Point<int> mouseDownPos)
{
	if (manager->managerFactory != nullptr)
	{
		manager->managerFactory->showCreateMenu([this, isFromAddButton, mouseDownPos](T* item)
			{
				this->addItemFromMenu(item, isFromAddButton, mouseDownPos);
			}
		);
	}
	else
	{
		if (isFromAddButton)
		{
			manager->BaseManager<T>::addItem();
			return;
		}

		PopupMenu p;
		p.addItem(1, addItemText);

		addMenuExtraItems(p, 2);

		p.showMenuAsync(PopupMenu::Options(), [this, isFromAddButton, mouseDownPos](int result)
			{
				if (result == 0) return;

				switch (result)
				{
				case 1:
					this->addItemFromMenu(isFromAddButton, mouseDownPos);
					break;

				default:
					this->handleMenuExtraItemsResult(result, 2);
					break;
				}
			}
		);
	}

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::addItemFromMenu(bool fromAddButton, Point<int> pos)
{
	addItemFromMenu(nullptr, fromAddButton, pos);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::addItemFromMenu(T* item, bool, Point<int>)
{
	manager->BaseManager<T>::addItem(item);
}

template<class M, class T, class U>
U* BaseManagerUI<M, T, U>::addItemUI(T* item, bool animate, bool resizeAndRepaint)
{
	U* tui = createUIForItem(item);

	BaseItemMinimalUI<T>* bui = static_cast<BaseItemMinimalUI<T>*>(tui);

	if (useViewport) container.addAndMakeVisible(bui);
	else addAndMakeVisible(bui);

	int index = manager->items.indexOf(item);
	itemsUI.insert(index, tui);

	bui->addItemMinimalUIListener(this);

	BaseItemUI<T>* biui = dynamic_cast<BaseItemUI<T>*>(tui);
	if (biui != nullptr) biui->addItemUIListener(this);


	addItemUIInternal(tui);

	if (animate && !Engine::mainEngine->isLoadingFile)
	{
		juce::Rectangle<int> tb = bui->getBounds();
		bui->setSize(10, 10);
		itemAnimator.animateComponent(bui, tb, 1, 200, false, 1, 0);
	}
	else
	{
		if (biui != nullptr && biui->baseItem->miniMode->boolValue()) biui->updateMiniModeUI();
		//DBG("resized");  
		//resized();
	}

	managerUIListeners.call(&ManagerUIListener::itemUIAdded, tui);

	repaint();

	return tui;
}

template<class M, class T, class U>
U* BaseManagerUI<M, T, U>::createUIForItem(T* item)
{
	return new U(item);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::removeItemUI(T* item, bool resizeAndRepaint)
{
	{
		MessageManagerLock mmLock; //Ensure this method can be called from another thread than the UI one

		U* tui = getUIForItem(item, false);
		if (tui == nullptr) return;

		BaseItemMinimalUI<T>* bui = static_cast<BaseItemMinimalUI<T>*>(tui);

		if (useViewport) container.removeChildComponent(bui);
		else removeChildComponent(bui);

		bui->removeItemMinimalUIListener(this);

		BaseItemUI<T>* biui = dynamic_cast<BaseItemUI<T>*>(tui);
		if (biui != nullptr) biui->removeItemUIListener(this);

		itemsUI.removeObject(tui, false);
		removeItemUIInternal(tui);

		managerUIListeners.call(&ManagerUIListener::itemUIRemoved, tui);

		delete tui;

		if (resizeAndRepaint)
		{
			resized();
			repaint();
		}
	}
}

template<class M, class T, class U>
U* BaseManagerUI<M, T, U>::getUIForItem(T* item, bool directIndexAccess)
{
	if (directIndexAccess) return itemsUI[static_cast<BaseManager<T>*>(manager)->items.indexOf(item)];

	for (auto& ui : itemsUI) if (static_cast<BaseItemMinimalUI<T>*>(ui)->item == item) return ui; //brute search, not needed if ui/items are synchronized
	return nullptr;
}

template<class M, class T, class U>
int BaseManagerUI<M, T, U>::getContentHeight()
{
	return container.getHeight() + 20;
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemAddedAsync(T* item)
{
	addItemUI(item, animateItemOnAdd);
	if (!animateItemOnAdd) resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemsAddedAsync(Array<T*> items)
{
	for (auto& i : items) addItemUI(i, false, false);

	resized();
	repaint();

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemRemoved(T* item)
{
	removeItemUI(item);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemsRemoved(Array<T*> items)
{
	if (items.size() == 0) return;

	for (auto& i : items) removeItemUI(i, false);

	MessageManagerLock mmLock;
	resized();
	repaint();

}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemsReorderedAsync()
{
	itemsUI.sort(managerComparator);
	resized();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::newMessage(const typename BaseManager<T>::ManagerEvent& e)
{
	switch (e.type)
	{
	case BaseManager<T>::ManagerEvent::ITEM_ADDED:
		if (e.getItem() == nullptr)
		{
			LOGWARNING("Item added but null");
			return;
		}
		itemAddedAsync(e.getItem());
		break;

	case BaseManager<T>::ManagerEvent::ITEMS_REORDERED:
		itemsReorderedAsync();
		break;

	case BaseManager<T>::ManagerEvent::ITEMS_ADDED:
		itemsAddedAsync(e.getItems());
		break;

	case BaseManager<T>::ManagerEvent::NEEDS_UI_UPDATE:
		repaint();
		//resized();
		break;

	default:
		break;
	}
}

template<class M, class T, class U>
bool BaseManagerUI<M, T, U>::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (acceptedDropTypes.contains(dragSourceDetails.description.getProperty("dataType", "").toString())) return true;

	U* itemUI = dynamic_cast<U*>(dragSourceDetails.sourceComponent.get());
	if (itemsUI.contains(itemUI)) return true;

	return false;
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	repaint();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemDragMove(const SourceDetails& dragSourceDetails)
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
	repaint();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::itemDropped(const SourceDetails& dragSourceDetails)
{

	if (defaultLayout == HORIZONTAL || defaultLayout == VERTICAL)
	{
		if (BaseItemMinimalUI<T>* bui = dynamic_cast<BaseItemMinimalUI<T>*>(dragSourceDetails.sourceComponent.get()))
		{
			if (T* item = bui->item)
			{
				int droppingIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
				if (itemsUI.contains((U*)bui))
				{
					if (itemsUI.indexOf((U*)bui) < droppingIndex) droppingIndex--;
					if (droppingIndex == -1) droppingIndex = itemsUI.size() - 1;
					this->manager->setItemIndex(item, droppingIndex);
				}
				else
				{
					var data = item->getJSONData();
					if (droppingIndex != -1) data.getDynamicObject()->setProperty("index", droppingIndex);

					if (T* newItem = this->manager->createItemFromData(data))
					{
						Array<UndoableAction*> actions;
						actions.add(this->manager->getAddItemUndoableAction(newItem, data));
						if (BaseManager<T>* sourceManager = dynamic_cast<BaseManager<T> *>(item->parentContainer.get()))
						{
							actions.addArray(sourceManager->getRemoveItemUndoableAction(item));
						}
						UndoMaster::getInstance()->performActions("Move " + item->niceName, actions);
					}
				}
			}
		}
	}

	this->isDraggingOver = false;
	repaint();
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::addSelectableComponentsAndInspectables(Array<Component*>& selectables, Array<Inspectable*>& inspectables)
{
	for (auto& i : itemsUI)
	{
		if (i->isVisible())
		{
			selectables.add(getSelectableComponentForItemUI(i));
			inspectables.add(i->inspectable);
		}
	}

}

template<class M, class T, class U>
Component* BaseManagerUI<M, T, U>::getSelectableComponentForItemUI(U* itemUI)
{
	return itemUI;
}

template<class M, class T, class U>
int BaseManagerUI<M, T, U>::getDropIndexForPosition(Point<int> localPosition)
{
	for (int i = 0; i < itemsUI.size(); ++i)
	{
		BaseItemMinimalUI<T>* iui = dynamic_cast<BaseItemMinimalUI<T>*>(itemsUI[i]);
		Point<int> p = getLocalArea(iui, iui->getLocalBounds()).getCentre();

		if (defaultLayout == HORIZONTAL && localPosition.x < p.x) return i;
		else if (defaultLayout == VERTICAL && localPosition.y < p.y) return i;
	}

	return -1;
}


template<class M, class T, class U>
void BaseManagerUI<M, T, U>::buttonClicked(Button* b)
{
	if (b == addItemBT.get())
	{
		showMenuAndAddItem(true, Point<int>());
	}
	else if (this->toolFuncMap.contains(b))
	{
		this->toolFuncMap[b]();
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::labelTextChanged(Label* l)
{
	if (l == searchBar.get())
	{
		resized();
	}
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::inspectableDestroyed(Inspectable*)
{
	if (manager != nullptr && !manager->isClearing)
		static_cast<BaseManager<T>*>(manager)->removeBaseManagerListener(this);
}

template<class M, class T, class U>
void BaseManagerUI<M, T, U>::newMessage(const Engine::EngineEvent& e)
{
	resized();
}
