/*
  ==============================================================================

	ManagerUI.h
	Created: 28 Oct 2016 8:03:45pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once
#pragma warning(disable:4244)



template<class M, class T>
class ManagerUI :
	public InspectableContentComponent,
	public Manager<T>::ManagerListener,
	public Manager<T>::AsyncListener,
	public juce::Button::Listener,
	public BaseItemUI::ItemUIListener,
	public BaseItemMinimalUI::ItemMinimalUIListener,
	public InspectableSelectionManager::AsyncListener,
	public juce::ComponentListener,
	public Engine::AsyncListener,
	public juce::DragAndDropTarget,
	public juce::TextEditor::Listener
{
public:

	static_assert(std::is_base_of<Manager<T>, M>::value, "M must be derived from Manager<T>");
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	ManagerUI(const juce::String& contentName, M* _manager, bool _useViewport = true);
	virtual ~ManagerUI();

	class ManagerViewport :
		public juce::Viewport
	{
	public:
		bool useMouseWheelMoveIfNeeded(const juce::MouseEvent& e, const juce::MouseWheelDetails& d) override
		{
			if (e.mods.isShiftDown()) return false;
			return juce::Viewport::useMouseWheelMoveIfNeeded(e, d);
		}
	};

	class ItemContainer :
		public juce::Component
	{
	public:
		ItemContainer(ManagerUI* mui) : managerUI(mui) {};
		~ItemContainer() {}

		ManagerUI* managerUI;

		void childBoundsChanged(juce::Component* c);
	};

	class ToolContainer : public juce::Component
	{
	public:
		ToolContainer() : juce::Component("Tools") {}
		void paint(juce::Graphics& g) override { g.setColour(BG_COLOR.darker(.6f)); g.fillRoundedRectangle(getLocalBounds().toFloat(), 2); }
	};
	ToolContainer toolContainer;
	juce::OwnedArray<juce::Component> tools;
	juce::HashMap<juce::Button*, std::function<void()>> toolFuncMap;
	bool showTools;

	M* manager;
	juce::OwnedArray<BaseItemMinimalUI> itemsUI;

	enum Layout { HORIZONTAL, VERTICAL, FREE };
	Layout defaultLayout;
	bool useViewport; //TODO, create a ManagerViewportUI


	int headerSize;
	juce::Colour bgColor;
	int labelHeight;
	int minHeight;
	juce::String managerUIName;
	bool drawContour;
	bool transparentBG;
	bool resizeOnChildBoundsChanged;
	bool autoFilterHitTestOnItems;
	bool validateHitTestOnNoItem;

	ManagerViewport viewport;
	ItemContainer container;

	std::unique_ptr<juce::ImageButton> addItemBT;
	std::unique_ptr<juce::TextEditor> searchBar;

	//ui
	juce::String noItemText;

	//menu
	juce::String addItemText;

	//Animation
	bool animateItemOnAdd;
	juce::ComponentAnimator itemAnimator;

	//selection
	bool useDedicatedSelector;
	bool selectingItems;

	//drag and drop
	juce::StringArray acceptedDropTypes;
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
	void addButtonTool(juce::Button* c, std::function<void()> clickFunc = nullptr);
	void addControllableTool(ControllableUI* c);

	virtual void paint(juce::Graphics& g) override;

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
		int compareElements(BaseItem* i1, BaseItem* i2) {
			return isVertical ? i2->viewUIPosition->y - i1->viewUIPosition->y : i2->viewUIPosition->x - i1->viewUIPosition->x;
		}
	};


	//Position and layout
	virtual void distributeItems(bool isVertical);

	virtual void updateItemsVisibility();
	virtual void updateItemVisibilityManagerInternal(BaseItemMinimalUI* bui);
	virtual void updateBaseItemVisibility(BaseItemMinimalUI* bui);

	virtual bool hasFiltering();
	virtual juce::Array<BaseItemMinimalUI*> getFilteredItems();
	virtual bool checkFilterForItem(BaseItemMinimalUI* item);


	//Add / Remove item UI
	virtual BaseItemMinimalUI* addItemUI(BaseItem* item, bool animate = false, bool resizeAndRepaint = true);
	virtual void addItemUIInternal(BaseItemMinimalUI* item) {}
	virtual void removeItemUI(BaseItem* item, bool resizeAndRepaint = true);
	virtual void removeItemUIInternal(BaseItemMinimalUI* item) {}

	virtual BaseItemMinimalUI* createUIForItem(BaseItem* item);
	BaseItemMinimalUI* getUIForItem(BaseItem* item, bool directIndexAccess = true);


	//Add item menu
	virtual void showMenuAndAddItem(bool isFromAddButton, juce::Point<int> mouseDownPos, std::function<void(BaseItem*)> callback = nullptr);
	virtual void addMenuExtraItems(juce::PopupMenu& p, int startIndex) {}
	virtual void handleMenuExtraItemsResult(int result, int startIndex) {}
	virtual void addItemFromMenu(BaseItem* item, bool isFromAddButton, juce::Point<int> mouseDownPos);

	//UI Events
	virtual void childBoundsChanged(juce::Component*) override;
	virtual bool hitTest(int x, int y) override;
	virtual void componentMovedOrResized(juce::Component& c, bool wasMoved, bool wasResized) override;


	//UI Interaction
	void buttonClicked(juce::Button* b) override;
	void textEditorTextChanged(juce::TextEditor& e) override;
	void textEditorReturnKeyPressed(juce::TextEditor& e) override;

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseUp(const juce::MouseEvent& e) override;


	//Selection
	virtual void askSelectToThis(BaseItemMinimalUI* item) override;


	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragMove(const SourceDetails& dragSourceDetails) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails& dragSourceDetails) override;

	//Selection
	virtual void addSelectableComponentsAndInspectables(juce::Array<juce::Component*>& selectables, juce::Array<Inspectable*>& inspectables);
	virtual juce::Component* getSelectableComponentForItemUI(BaseItemMinimalUI* itemUI);

	virtual int getDropIndexForPosition(juce::Point<int> localPosition);
	virtual void itemUIMiniModeChanged(BaseItemUI* se) override {}

	//Helpers
	int getContentHeight();


	//Manager async events
	virtual void itemAddedAsync(BaseItem* item);
	virtual void itemsAddedAsync(juce::Array<BaseItem*> items);
	virtual void itemRemovedAsync(BaseItem* item);
	virtual void itemsRemovedAsync(juce::Array<BaseItem*> items);
	virtual void itemsReorderedAsync();


	//ManagerUI Events
	virtual void notifyItemUIAdded(BaseItemMinimalUI* ui);
	virtual void notifyItemUIRemoved(BaseItemMinimalUI* ui);


	//Inspectable and selection
	virtual void inspectableDestroyed(Inspectable*) override;
	virtual void newMessage(const Engine::EngineEvent& e);
	virtual void newMessage(const InspectableSelectionManager::SelectionEvent& e) override;
	virtual void newMessage(const ManagerEvent<T>& e) override;


	class  ManagerUIListener
	{
	public:
		/** Destructor. */
		virtual ~ManagerUIListener() {}
		virtual void itemUIAdded(BaseItemMinimalUI*) {}
		virtual void itemUIRemoved(BaseItemMinimalUI*) {}
	};

	juce::ListenerList<ManagerUIListener> managerUIListeners;
	void addManagerUIListener(ManagerUIListener* newListener) { managerUIListeners.add(newListener); }
	void removeManagerUIListener(ManagerUIListener* listener) { managerUIListeners.remove(listener); }
};





//IMPLEMENTATION

template<class M, class T>
ManagerUI<M, T>::ManagerUI(const juce::String& contentName, M* _manager, bool _useViewport) :
	InspectableContentComponent(_manager),
	manager(_manager),
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
		//viewport.setEnableKeyPressEvents(false);
		this->addAndMakeVisible(viewport);
	}

	addItemBT.reset(AssetManager::getInstance()->getAddBT());
	addItemBT->setWantsKeyboardFocus(false);

	addAndMakeVisible(addItemBT.get());
	addItemBT->addListener(this);

	setShowAddButton(manager->userCanAddItemsManually);
	acceptedDropTypes.add(manager->itemDataType);

	Engine::mainEngine->addAsyncEngineListener(this);

	InspectableSelectionManager::mainSelectionManager->addAsyncSelectionManagerListener(this);
	//must call addExistingItems from child class to get overrides

	setWantsKeyboardFocus(true);


	manager->addManagerListener(this, false);
	manager->addAsyncManagerListener(this, false);
}


template<class M, class T>
ManagerUI<M, T>::~ManagerUI()
{
	if (!inspectable.wasObjectDeleted())
	{
		this->manager->removeManagerListener(this);
		this->manager->removeAsyncManagerListener(this);
	}

	Engine::mainEngine->removeAsyncEngineListener(this);
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeAsyncSelectionManagerListener(this);

}

template<class M, class T>
BaseItemMinimalUI* ManagerUI<M, T>::createUIForItem(BaseItem* item)
{
	if (item == nullptr) return nullptr;
	return item->createUI();
}


template<class M, class T>
void ManagerUI<M, T>::showMenuAndAddItem(bool isFromAddButton, juce::Point<int> mouseDownPos, std::function<void(BaseItem*)> callback)
{
	if (manager->managerFactory != nullptr)
	{
		manager->managerFactory->showCreateMenu([this, isFromAddButton, mouseDownPos, callback](BaseItem* item)
			{
				this->addItemFromMenu(item, isFromAddButton, mouseDownPos);
				if (callback != nullptr) callback(item);

			}
		);
		return;
	}

	if (isFromAddButton)
	{
		BaseItem* item = manager->Manager<T>::addItem();
		if (callback != nullptr) callback(item);
		return;
	}

	this->addItemFromMenu(nullptr, isFromAddButton, mouseDownPos);
}


template<class M, class T>
juce::Component* ManagerUI<M, T>::getSelectableComponentForItemUI(BaseItemMinimalUI* itemUI)
{
	return itemUI;
}


template<class M, class T>
void ManagerUI<M, T>::setDefaultLayout(Layout l)
{
	defaultLayout = l;
	if (useViewport)
	{
		if (defaultLayout == VERTICAL) viewport.setScrollBarsShown(true, false);
		else viewport.setScrollBarsShown(false, true);
	}

}

template<class M, class T>
void ManagerUI<M, T>::addExistingItems(bool resizeAfter)
{

	//add existing items
	juce::Array<T*> items = manager->getItems();
	for (auto& t : items) addItemUI(t, false);
	if (resizeAfter) resized();
}

template<class M, class T>
void ManagerUI<M, T>::setShowAddButton(bool value)
{
	addItemBT->setVisible(value);

	if (value) addAndMakeVisible(addItemBT.get());
	else removeChildComponent(addItemBT.get());
}

template<class M, class T>
void ManagerUI<M, T>::setShowSearchBar(bool value)
{
	if (value)
	{
		searchBar.reset(new TextEditor("SearchBar"));
		searchBar->setSelectAllWhenFocused(true);

		//searchBar->setJustificationType(Justification::topLeft);
		searchBar->setColour(searchBar->backgroundColourId, BG_COLOR.darker(.1f).withAlpha(.7f));
		searchBar->setColour(searchBar->outlineColourId, BG_COLOR.brighter(.1f));
		searchBar->setColour(searchBar->textColourId, TEXT_COLOR.darker(.3f));
		searchBar->setFont(FontOptions(10));
		searchBar->setMultiLine(false);
		searchBar->setColour(CaretComponent::caretColourId, Colours::orange);
		//searchBar->edit(true);
		searchBar->addListener(this);
		searchBar->setTextToShowWhenEmpty("Search...", TEXT_COLOR.withAlpha(.6f));
		addAndMakeVisible(searchBar.get());
	}
	else if (searchBar != nullptr)
	{
		removeChildComponent(searchBar.get());
		searchBar.reset();
	}
}

template<class M, class T>
void ManagerUI<M, T>::setShowTools(bool value)
{
	if (value == showTools) return;

	showTools = value;
	if (value)
	{
		addAndMakeVisible(&toolContainer);
	}
	else
	{
		toolContainer.setVisible(false);
		removeChildComponent(&toolContainer);
	}

	resized();
}

template<class M, class T>
void ManagerUI<M, T>::addButtonTool(juce::Button* c, std::function<void()> clickFunc)
{
	if (tools.contains(c)) return;
	tools.add(c);
	toolContainer.addAndMakeVisible(c);
	if (clickFunc != nullptr) toolFuncMap.set(c, clickFunc);
	c->addListener(this);
}

template<class M, class T>
void ManagerUI<M, T>::addControllableTool(ControllableUI* c)
{
	toolContainer.addAndMakeVisible(c);
	tools.add(c);
}

template<class M, class T>
void ManagerUI<M, T>::mouseDown(const MouseEvent& e)
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
					juce::Array<Component*> selectables;
					juce::Array<Inspectable*> inspectables;
					addSelectableComponentsAndInspectables(selectables, inspectables);

					InspectableSelector::getInstance()->startSelection(this, selectables, inspectables, nullptr, !e.mods.isShiftDown());
					selectingItems = true;
				}
			}
		}
		else if (e.mods.isRightButtonDown())
		{
			if (manager->userCanAddItemsManually) showMenuAndAddItem(false, e.getEventRelativeTo(this).getMouseDownPosition());
		}
	}

}

template<class M, class T>
void ManagerUI<M, T>::mouseUp(const juce::MouseEvent& e)
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

template<class M, class T>
void ManagerUI<M, T>::askSelectToThis(BaseItemMinimalUI* itemUI)
{
	BaseItem* firstItem = InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>();
	int firstIndex = manager->getItemIndex(firstItem);
	int itemIndex = manager->getItemIndex(itemUI->baseItem);

	if (firstIndex == itemIndex) return;

	if (firstIndex >= 0 && itemIndex >= 0)
	{
		int step = firstIndex < itemIndex ? 1 : -1;

		for (int index = firstIndex; index != itemIndex; index += step)
		{
			manager->getItemAt(index)->selectThis(true);
		}

	}

	if (itemIndex >= 0) manager->getItemAt(itemIndex)->selectThis(true);
}


template<class M, class T>
void ManagerUI<M, T>::paint(juce::Graphics& g)
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
		float textWidth = (int)TextLayout::getStringWidth(g.getCurrentFont(), managerUIName);
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
				BaseItemMinimalUI* bui = dynamic_cast<BaseItemMinimalUI*>(itemsUI[currentDropIndex >= 0 ? currentDropIndex : itemsUI.size() - 1]);
				if (bui != nullptr)
				{
					juce::Rectangle<int> buiBounds = getLocalArea(bui, bui->getLocalBounds());
					if (defaultLayout == HORIZONTAL)
					{
						int tx = currentDropIndex >= 0 ? buiBounds.getX() - 1 : buiBounds.getRight() + 1;
						g.drawLine(tx, getHeight() * .25f, tx, getHeight() * .75f, 2);
					}
					else if (defaultLayout == VERTICAL)
					{
						int ty = currentDropIndex >= 0 ? buiBounds.getY() - 1 : buiBounds.getBottom() + 1;
						g.drawLine(getWidth() * .25f, ty, getWidth() * .75f, ty, 2);
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


	if (!this->inspectable.wasObjectDeleted() && this->manager->hasNoItems() && noItemText.isNotEmpty())
	{
		g.setColour(Colours::white.withAlpha(.4f));
		g.setFont(FontOptions(jmin(getHeight() - 2, 14)));
		g.drawFittedText(noItemText, getLocalBounds().reduced(5), Justification::centred, 6);
	}
}

template<class M, class T>
void ManagerUI<M, T>::resized()
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

template<class M, class T>
juce::Rectangle<int> ManagerUI<M, T>::setHeaderBounds(juce::Rectangle<int>& r)
{
	return defaultLayout == VERTICAL ? r.removeFromTop(headerSize) : r.removeFromRight(headerSize);
}

template<class M, class T>
void ManagerUI<M, T>::resizedInternalHeader(juce::Rectangle<int>& hr)
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

template<class M, class T>
void ManagerUI<M, T>::resizedInternalHeaderTools(juce::Rectangle<int>& r)
{
	r.removeFromLeft(4);

	float rSize = 0;
	for (auto& t : this->tools) rSize += jmax(t->getWidth(), r.getHeight());

	r.removeFromLeft((r.getWidth() - rSize) / 2);

	if (r.getWidth() == 0 || r.getHeight() == 0) return;

	toolContainer.setBounds(r.withWidth(rSize));

	juce::Rectangle<int> toolR = toolContainer.getLocalBounds();

	for (auto& t : this->tools)
	{
		juce::Rectangle<int> tr = toolR.removeFromLeft(jmax(t->getWidth(), r.getHeight()));
		tr.reduce(tr.getWidth() == toolR.getHeight() ? 6 : 0, 6);
		t->setBounds(tr);
	}
}

template<class M, class T>
void ManagerUI<M, T>::resizedInternalContent(juce::Rectangle<int>& r)
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
			if (itemsUI.size() > 0) th = itemsUI[itemsUI.size() - 1]->getBottom();
			//if (grabbingItem != nullptr) th = jmax<int>(th + grabbingItem->getHeight(), viewport.getHeight());

			if (useViewport) container.setSize(getWidth(), th);
			else this->setSize(getWidth(), juce::jmax<int>(th + 10, minHeight));
		}
		else if (defaultLayout == HORIZONTAL)
		{
			float tw = 0;
			if (itemsUI.size() > 0) tw = itemsUI[itemsUI.size() - 1]->getRight();
			//if (grabbingItem != nullptr) tw = jmax<int>(tw, viewport.getWidth());
			if (useViewport) container.setSize(tw, getHeight());
			else this->setSize(tw, getHeight());
		}
	}
}

template<class M, class T>
void ManagerUI<M, T>::placeItems(juce::Rectangle<int>& r)
{
	int i = 0;
	for (auto& ui : itemsUI)
	{
		if (!checkFilterForItem(ui))
		{
			ui->setVisible(false);
			continue;
		}
		ui->setVisible(true);

		juce::Rectangle<int> tr;
		if (defaultLayout == VERTICAL) tr = r.withHeight(ui->getHeight());
		else tr = r.withWidth(ui->getWidth());

		if (tr != ui->getBounds()) ui->setBounds(tr);

		if (defaultLayout == VERTICAL) r.translate(0, tr.getHeight() + gap);
		else r.translate(tr.getWidth() + gap, 0);

		++i;
	}
}

template<class M, class T>
void ManagerUI<M, T>::resizedInternalFooter(juce::Rectangle<int>& r)
{
}

template<class M, class T>
void ManagerUI<M, T>::alignItems(AlignMode alignMode)
{
	juce::Array<BaseItem*> inspectables = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
	if (inspectables.size() == 0) return;

	float target = (alignMode == AlignMode::CENTER_V || alignMode == AlignMode::CENTER_H) ? 0 : ((alignMode == AlignMode::LEFT || alignMode == AlignMode::TOP) ? INT32_MAX : INT32_MIN);

	juce::Array<BaseItem*> goodInspectables;
	for (auto& i : inspectables)
	{
		if (i == nullptr || !manager->hasItem(i)) continue;
		switch (alignMode)
		{
		case AlignMode::LEFT: target = jmin(i->viewUIPosition->x, target); break;
		case AlignMode::RIGHT: target = jmax(i->viewUIPosition->x + i->viewUISize->x, target); break;
		case AlignMode::CENTER_H: target += i->viewUIPosition->x + i->viewUISize->x / 2; break;
		case AlignMode::TOP: target = jmin(target, i->viewUIPosition->y); break;
		case AlignMode::BOTTOM: target = jmax(target, i->viewUIPosition->y + i->viewUISize->y); break;
		case AlignMode::CENTER_V: target += i->viewUIPosition->y + i->viewUISize->y / 2; break;
		}
		goodInspectables.add(i);
	}

	if (goodInspectables.size() == 0) return;

	if (alignMode == CENTER_H || alignMode == CENTER_V) target /= goodInspectables.size();

	juce::Array<UndoableAction*> actions;
	for (auto& i : goodInspectables)
	{
		Point<float> targetPoint;
		switch (alignMode)
		{
		case AlignMode::LEFT: targetPoint = Point<float>(target, i->viewUIPosition->y); break;
		case AlignMode::RIGHT: targetPoint = Point<float>(target - i->viewUISize->x, i->viewUIPosition->y); break;
		case AlignMode::CENTER_H: targetPoint = Point<float>(target - i->viewUISize->x / 2, i->viewUIPosition->y);  break;
		case AlignMode::TOP: targetPoint = Point<float>(i->viewUIPosition->x, target); break;
		case AlignMode::BOTTOM:targetPoint = Point<float>(i->viewUIPosition->x, target - i->viewUISize->y); break;
		case AlignMode::CENTER_V:targetPoint = Point<float>(i->viewUIPosition->x, target - i->viewUISize->y / 2); break;
		}
		actions.addArray(i->viewUIPosition->setUndoablePoint(targetPoint, true));
	}

	UndoMaster::getInstance()->performActions("Align " + String(goodInspectables.size()) + " items", actions);

}

template<class M, class T>
void ManagerUI<M, T>::distributeItems(bool isVertical)
{
	juce::Array<BaseItem*> inspectables = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
	if (inspectables.size() < 3) return;


	juce::Array<BaseItem*> goodInspectables;
	for (auto& i : inspectables)
	{
		if (i == nullptr || !manager->hasItem(i)) continue;

		goodInspectables.add(i);
	}

	if (goodInspectables.size() < 3) return;


	PositionComparator comp(isVertical);
	goodInspectables.sort(comp);

	Point<float> center0 = goodInspectables[0]->viewUIPosition->getPoint() + goodInspectables[0]->viewUISize->getPoint() / 2;
	Point<float> center1 = goodInspectables[goodInspectables.size() - 1]->viewUIPosition->getPoint() + goodInspectables[goodInspectables.size() - 1]->viewUISize->getPoint() / 2;
	float tMin = isVertical ? center0.y : center0.x;
	float tMax = isVertical ? center1.y : center1.x;

	juce::Array<UndoableAction*> actions;
	for (int i = 0; i < goodInspectables.size(); i++)
	{
		float rel = i * 1.0f / (goodInspectables.size() - 1);
		float target = jmap(rel, tMin, tMax);
		BaseItem* ti = goodInspectables[i];
		Point<float> targetPoint(isVertical ? ti->viewUIPosition->x : target - ti->viewUISize->x / 2, isVertical ? target - ti->viewUISize->y / 2 : ti->viewUIPosition->y);
		actions.addArray(ti->viewUIPosition->setUndoablePoint(targetPoint, true));
	}

	UndoMaster::getInstance()->performActions("Distribute " + String(goodInspectables.size()) + " items", actions);
}


template<class M, class T>
void ManagerUI<M, T>::updateItemsVisibility()
{
	for (auto& bui : itemsUI) updateItemVisibilityManagerInternal(bui);

}

template<class M, class T>
void ManagerUI<M, T>::updateItemVisibilityManagerInternal(BaseItemMinimalUI* bui)
{
	updateBaseItemVisibility(bui);
}

template<class M, class T>
void ManagerUI<M, T>::updateBaseItemVisibility(BaseItemMinimalUI* bui)
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

template<class M, class T>
bool ManagerUI<M, T>::hasFiltering()
{
	return searchBar != nullptr && searchBar->getText().isNotEmpty();
}

template<class M, class T>
juce::Array<BaseItemMinimalUI*> ManagerUI<M, T>::getFilteredItems()
{
	if (!this->hasFiltering()) return juce::Array<BaseItemMinimalUI*>(this->itemsUI.getRawDataPointer(), this->itemsUI.size());

	juce::Array<BaseItemMinimalUI*> result;
	for (auto& ui : this->itemsUI) if (checkFilterForItem(ui)) result.add(ui);
	return result;
}

template<class M, class T>
bool ManagerUI<M, T>::checkFilterForItem(BaseItemMinimalUI* ui)
{
	if (!this->hasFiltering() || searchBar == nullptr) return true;
	return ui->baseItem->niceName.toLowerCase().contains(searchBar->getText().toLowerCase());
}

template<class M, class T>
void ManagerUI<M, T>::childBoundsChanged(Component* c)
{
	if (resizeOnChildBoundsChanged && c != &viewport) resized();
}

template<class M, class T>
bool ManagerUI<M, T>::hitTest(int x, int y)
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

template<class M, class T>
void ManagerUI<M, T>::componentMovedOrResized(Component& c, bool wasMoved, bool wasResized)
{
	if (&c == &container && useViewport && !itemAnimator.isAnimating())
	{
		resized();
	}
}

template<class M, class T>
void ManagerUI<M, T>::addItemFromMenu(BaseItem* item, bool isFromAddButton, juce::Point<int> mouseDownPos)
{
	if (item != nullptr)
	{
		manager->Manager<T>::addItem(item);
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
				this->addItemFromMenu(manager->createItem(), isFromAddButton, mouseDownPos);
				break;

			default:
				this->handleMenuExtraItemsResult(result, 2);
				break;
			}
		}
	);
}


template<class M, class T>
BaseItemMinimalUI* ManagerUI<M, T>::addItemUI(BaseItem* item, bool animate, bool resizeAndRepaint)
{
	if (item == nullptr) return nullptr;
	BaseItemMinimalUI* tui = createUIForItem(item);
	jassert(tui != nullptr);

	if (useViewport) container.addAndMakeVisible(tui);
	else addAndMakeVisible(tui);

	int index = manager->getItemIndex(item);
	itemsUI.insert(index, tui);

	tui->addItemMinimalUIListener(this);

	BaseItemUI* biui = dynamic_cast<BaseItemUI*>(tui);
	if (biui != nullptr) biui->addItemUIListener(this);

	this->addItemUIInternal(tui);

	if (animate && !Engine::mainEngine->isLoadingFile)
	{
		juce::Rectangle<int> tb = tui->getBounds();
		tui->setSize(10, 10);
		itemAnimator.animateComponent(tui, tb, 1, 200, false, 1, 0);
	}
	else
	{
		if (biui != nullptr && biui->baseItem->miniMode->boolValue()) biui->updateMiniModeUI();
		//DBG("resized");  
		//resized();
	}

	notifyItemUIAdded(tui);

	if (resizeAndRepaint)
	{
		resized();
		repaint();
	}

	return tui;
}

template<class M, class T>
void ManagerUI<M, T>::removeItemUI(BaseItem* item, bool resizeAndRepaint)
{
	{
		if (!MessageManager::getInstance()->isThisTheMessageThread())
		{
			MessageManager::getInstance()->callAsync([this, item, resizeAndRepaint]()
				{
					removeItemUI(item, resizeAndRepaint);
				});
			return;
		}

		//MessageManagerLock mmLock; //Ensure this method can be called from another thread than the UI one

		BaseItemMinimalUI* tui = getUIForItem(item, false);
		if (tui == nullptr) return;


		if (useViewport) container.removeChildComponent(tui);
		else removeChildComponent(tui);

		tui->removeItemMinimalUIListener(this);

		BaseItemUI* biui = dynamic_cast<BaseItemUI*>(tui);
		if (biui != nullptr) biui->removeItemUIListener(this);

		itemsUI.removeObject(tui, false);
		removeItemUIInternal(tui);

		notifyItemUIRemoved(tui);

		delete tui;

		if (resizeAndRepaint)
		{
			resized();
			repaint();
		}
	}
}

template<class M, class T>
BaseItemMinimalUI* ManagerUI<M, T>::getUIForItem(BaseItem* item, bool directIndexAccess)
{
	if (directIndexAccess) return itemsUI[manager->getItemIndex(item)];

	for (auto& ui : itemsUI) if (ui->baseItem == item) return ui; //brute search, not needed if ui/items are synchronized
	return nullptr;
}

template<class M, class T>
int ManagerUI<M, T>::getContentHeight()
{
	return container.getHeight() + 20;
}


template<class M, class T>
void ManagerUI<M, T>::itemAddedAsync(BaseItem* item)
{
	addItemUI(item, animateItemOnAdd);
	if (!animateItemOnAdd) resized();
}

template<class M, class T>
void ManagerUI<M, T>::itemsAddedAsync(juce::Array<BaseItem*> items)
{
	for (auto& i : items) addItemUI(i, false, false);

	resized();
	repaint();
}


template<class M, class T>
void ManagerUI<M, T>::itemRemovedAsync(BaseItem* item)
{
	removeItemUI(item);
}

template<class M, class T>
void ManagerUI<M, T>::itemsRemovedAsync(juce::Array<BaseItem*> items)
{
	if (items.size() == 0) return;

	for (auto& i : items) removeItemUI(i, false);

	juce::MessageManager::getInstance()->callAsync([this]()
		{
			resized();
			repaint();
		});

}

template<class M, class T>
void ManagerUI<M, T>::itemsReorderedAsync()
{
	//Rebuild all UIs
	Array<T*> items = manager->getItems();
	for (auto& i : items)
	{
		BaseItemMinimalUI* ui = getUIForItem(i, true);
		int uiIndex = itemsUI.indexOf(ui);
		int dataIndex = items.indexOf(i);
		if (uiIndex == dataIndex || ui == nullptr) continue;
		itemsUI.move(uiIndex, dataIndex);
	}
	resized();
}

template<class M, class T>
void ManagerUI<M, T>::newMessage(const InspectableSelectionManager::SelectionEvent& e)
{
	if (e.type == e.SELECTION_CHANGED)
	{
		if (useViewport)
		{
			//autofocus ?
		}
	}
}

template<class M, class T>
bool ManagerUI<M, T>::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (acceptedDropTypes.contains(dragSourceDetails.description.getProperty("dataType", "").toString())) return true;

	BaseItemMinimalUI* itemUI = dynamic_cast<BaseItemMinimalUI*>(dragSourceDetails.sourceComponent.get());
	if (itemsUI.contains(itemUI)) return true;

	return false;
}

template<class M, class T>
void ManagerUI<M, T>::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	repaint();
}

template<class M, class T>
void ManagerUI<M, T>::itemDragMove(const SourceDetails& dragSourceDetails)
{
	if (defaultLayout == HORIZONTAL || defaultLayout == VERTICAL)
	{
		currentDropIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
		repaint();
	}
}


template<class M, class T>
void ManagerUI<M, T>::itemDragExit(const SourceDetails&)
{
	isDraggingOver = false;
	repaint();
}

template<class M, class T>
void ManagerUI<M, T>::itemDropped(const SourceDetails& dragSourceDetails)
{

	if (defaultLayout == HORIZONTAL || defaultLayout == VERTICAL)
	{
		if (BaseItemMinimalUI* bui = dynamic_cast<BaseItemMinimalUI*>(dragSourceDetails.sourceComponent.get()))
		{
			if (BaseItem* item = bui->baseItem)
			{
				int droppingIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
				if (itemsUI.contains(bui))
				{
					if (itemsUI.indexOf(bui) < droppingIndex) droppingIndex--;
					if (droppingIndex == -1) droppingIndex = itemsUI.size() - 1;
					this->manager->setItemIndex(item, droppingIndex);
				}
				else
				{
					var data = item->getJSONData();
					if (droppingIndex != -1) data.getDynamicObject()->setProperty("index", droppingIndex);

					if (BaseItem* newItem = this->manager->createItemFromData(data))
					{
						juce::Array<UndoableAction*> actions;
						actions.add(this->manager->getAddItemUndoableAction(newItem, data));
						if (M* sourceManager = dynamic_cast<M*>(item->parentContainer.get()))
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

template<class M, class T>
void ManagerUI<M, T>::addSelectableComponentsAndInspectables(juce::Array<Component*>& selectables, juce::Array<Inspectable*>& inspectables)
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

template<class M, class T>
juce::Component* ManagerUI<M, T>::getSelectableComponentForItemUI(BaseItemMinimalUI* itemUI)
{
	return itemUI;
}

template<class M, class T>
int ManagerUI<M, T>::getDropIndexForPosition(juce::Point<int> localPosition)
{
	for (int i = 0; i < itemsUI.size(); ++i)
	{
		BaseItemMinimalUI* iui = dynamic_cast<BaseItemMinimalUI*>(itemsUI[i]);
		juce::Point<int> p = getLocalArea(iui, iui->getLocalBounds()).getCentre();

		if (defaultLayout == HORIZONTAL && localPosition.x < p.x) return i;
		else if (defaultLayout == VERTICAL && localPosition.y < p.y) return i;
	}

	return -1;
}


template<class M, class T>
void ManagerUI<M, T>::buttonClicked(juce::Button* b)
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

template<class M, class T>
void ManagerUI<M, T>::textEditorTextChanged(juce::TextEditor& e)
{
	if (&e == searchBar.get())
	{
		resized();
	}
}

template<class M, class T>
void ManagerUI<M, T>::textEditorReturnKeyPressed(juce::TextEditor& e)
{
	if (&e == searchBar.get())
	{
		searchBar->unfocusAllComponents();
	}
}

template<class M, class T>
void ManagerUI<M, T>::inspectableDestroyed(Inspectable*)
{
	//to be overriden in templated class
}

template<class M, class T>
void ManagerUI<M, T>::newMessage(const Engine::EngineEvent& e)
{
	resized();
}

template<class M, class T>
void ManagerUI<M, T>::ItemContainer::childBoundsChanged(juce::Component* c) {
	managerUI->childBoundsChanged(c);
}

template<class M, class T>
void ManagerUI<M, T>::newMessage(const typename ManagerEvent<T>& e)
{
	switch (e.type)
	{
	case Manager<T>::ManagerEventType::ITEM_ADDED:
		itemAdded(e.getItems().getFirst());
		break;

	case  Manager<T>::ManagerEventType::ITEMS_ADDED:
		itemsAddedAsync(e.getItems());
		break;

	case  Manager<T>::ManagerEventType::ITEM_REMOVED:
		itemRemovedAsync(e.getItems().getFirst());
		break;

	case  Manager<T>::ManagerEventType::ITEMS_REMOVED:
		itemsRemovedAsync(e.getItems());
		break;

	case  Manager<T>::ManagerEventType::ITEMS_REORDERED:
		itemsReorderedAsync();
		break;

	case  Manager<T>::ManagerEventType::NEEDS_UI_UPDATE:
		repaint();
		resized();
		break;

	default:
		break;
	}
}

template<class M, class T>
void ManagerUI<M, T>::inspectableDestroyed(Inspectable*)
{
	if (manager != nullptr && !manager->isClearing)
		static_cast<Manager<T>*>(manager)->removeManagerListener(this);
}

template<class M, class T>
void ManagerUI<M, T>::notifyItemUIAdded(BaseItemMinimalUI* itemUI)
{
	managerUIListeners.call(&ManagerUIListener::itemUIAdded, itemUI);
}

template<class M, class T>
void ManagerUI<M, T>::notifyItemUIRemoved(BaseItemMinimalUI* itemUI)
{
	managerUIListeners.call(&ManagerUIListener::itemUIRemoved, itemUI);
}
